#include "AudioMediaUdsWriter.hxx"

#include <assert.h>
#include <errno.h>
#include <error.h>
#include <memory.h>
#include <string.h>

#include <sys/socket.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

#define THIS_FILE "AudioMediaUdsWriter.cxx"

using namespace std;
using namespace pj;

namespace sipxsua {

AudioMediaUdsWriter::AudioMediaUdsWriter() : AudioMedia() {
  id == PJSUA_INVALID_ID;
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "AudioMediaUdsWriter", 8192, 8192,
                        NULL);
}

AudioMediaUdsWriter::~AudioMediaUdsWriter() {
  if (id != PJSUA_INVALID_ID) {
    unregisterMediaPort();
  }
  if (sockfd != 0) {
    close(sockfd);
  }
  if (NULL != src_state) {
    src_delete(src_state);
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

void AudioMediaUdsWriter::createRecorder(
    const pj::MediaFormatAudio &audioFormat, const string &sendtoPath,
    unsigned sampleRate, unsigned bufferMSec) {
  assert(id == PJSUA_INVALID_ID); // 不允许重复创建！！！

  // 只支持 Mono channel
  assert(audioFormat.channelCount == 1);
  // 只支持 16bits sampling
  assert(audioFormat.bitsPerSample == 16);

  this->audioFormat = audioFormat;
  this->sampleRate = sampleRate;

  // 远端的声音媒体作为 source，获取它的格式规格
  // 计算缓冲区大小
  size_t bytes_per_sec = audioFormat.clockRate * audioFormat.channelCount *
                         audioFormat.bitsPerSample / 8; // 每秒的字节数
  buffer_size = bytes_per_sec * bufferMSec / 1000;
  unsigned int samples_per_frame = audioFormat.clockRate *
                                   audioFormat.channelCount *
                                   audioFormat.frameTimeUsec / 1000000;

  //分配缓冲区
  buffer = (uint8_t *)pj_pool_calloc(pool, buffer_size, sizeof(uint8_t));

  // 打开 Unix domain socket
  sendto_addr = (sockaddr_un *)pj_pool_calloc(pool, 1, sizeof(sockaddr_un));
  sendto_addr->sun_family = AF_LOCAL;
  strncpy(sendto_addr->sun_path, sendtoPath.c_str(),
          sizeof(sendto_addr->sun_path) - 1);

  sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    ostringstream oss;
    oss << "socket error (" << errno << "): " << strerror(errno);
    cerr << oss.str() << endl;
    throw new runtime_error(oss.str());
  }

  // 新建 resampler
  if (sampleRate != audioFormat.clockRate) {

    int src_err = 0;
    src_state =
        src_new(SRC_SINC_MEDIUM_QUALITY, audioFormat.channelCount, &src_err);
    if (src_err) {
      ostringstream oss;
      oss << "samplerate src_new() error (" << src_err << ") "
          << src_strerror(src_err) << endl;
      cerr << oss.str() << endl;
      throw new runtime_error(oss.str());
    }
    // 重采样率比例：output_sample_rate / input_sample_rate
    src_data.src_ratio = (double)sampleRate / (double)audioFormat.clockRate;
    // 计算输入数据的 frame(采样!) 个数！ (PJ一般是16bit) 的 PCM
    src_data.input_frames = audioFormat.clockRate * bufferMSec / 1000;
    // 计算输出数据的 frame(采样!)个数
    src_data.output_frames =
        src_data.input_frames * sampleRate / audioFormat.clockRate;
    // 分配输入缓冲
    src_data.data_in =
        (float *)pj_pool_calloc(pool, src_data.input_frames, sizeof(float));
    // 分配输出缓冲
    src_data.data_out =
        (float *)pj_pool_calloc(pool, src_data.output_frames, sizeof(float));
    // 分配 short 数组输出缓冲
    resampled_short_array =
        (short *)pj_pool_calloc(pool, src_data.output_frames, sizeof(short));
  }

  // 建立内存采集 Audio Port
  pjmedia_mem_capture_create(pool, buffer, buffer_size, audioFormat.clockRate,
                             audioFormat.channelCount, samples_per_frame,
                             audioFormat.bitsPerSample, 0, &port);
  // C++ way： 把 Port 加入到 conf，并接收新的 port id 到这个类的 id 属性
  registerMediaPort2(port, pool);
  // 如果上面一步失败，就不会产生有效的 media id.
  // 接收回调
  pjmedia_mem_capture_set_eof_cb2(port, (void *)this, cb_mem_capture_eof);
}

void AudioMediaUdsWriter::onBufferEof() {
  // 如果不用 resample，缓冲的数据可以直接发送
  uint8_t *send_buf = buffer;
  size_t send_sz = buffer_size;
  // Resample?
  if (NULL != src_state) {
    // 重采样！
    int src_err = 0;
    // The src_reset function resets the internal state of the sample rate
    // converter object to the same state it had immediately after its creation
    // using src_new. This should be called whenever a sample rate converter is
    // to be used on two separate, unrelated pieces of audio.
    src_err = src_reset(src_state);
    if (src_err) {
      ostringstream oss;
      oss << "samplerate src_reset() error (" << src_err << ") "
          << src_strerror(src_err) << endl;
      cerr << oss.str() << endl;
      throw new runtime_error(oss.str());
    }
    // 原始16bit采样数据转float采样数据装载到输入缓冲
    src_short_to_float_array((const short *)buffer, (float *)src_data.data_in,
                             src_data.input_frames);
    // 重采样处理：输入=>输出
    do {
      src_err = src_process(src_state, &src_data);
      if (src_err) {
        ostringstream oss;
        oss << "samplerate src_process() error (" << src_err << ") "
            << src_strerror(src_err) << endl;
        cerr << oss.str() << endl;
        throw new runtime_error(oss.str());
      }
    } while (src_data.output_frames_gen < src_data.output_frames);
    // 分配结果数据缓冲区，并将输出float采样的缓冲数据该写成16Kbit采样的数据写入到结果缓冲
    src_float_to_short_array(src_data.data_out, resampled_short_array,
                             src_data.output_frames);
    // 把发送缓冲指向到这里!
    send_buf = (uint8_t *)resampled_short_array;
    send_sz = src_data.output_frames * sizeof(short);
  }

  // 发送!
  ssize_t sent_bytes =
      sendto(sockfd, send_buf, send_sz, 0, (struct sockaddr *)sendto_addr,
             sizeof(*sendto_addr));
  if (sent_bytes < 0) {
    switch (errno) {
    case ENOENT:
      break;
    case ECONNREFUSED:
      break;
    default: {
      PJ_LOG(2, ("AudioMediaUdsWriter", "UDS data gram send error (%d): %s",
                 errno, strerror(errno)));
    } break;
    }
  }
}

void AudioMediaUdsWriter::cb_mem_capture_eof(pjmedia_port *port,
                                             void *usr_data) {
  ((AudioMediaUdsWriter *)usr_data)->onBufferEof();
}

} // namespace sipxsua