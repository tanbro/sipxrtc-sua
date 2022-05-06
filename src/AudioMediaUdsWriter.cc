#include "AudioMediaUdsWriter.hh"

#include <sys/socket.h>

#include <glog/logging.h>

using namespace std;
using namespace pj;

#define THIS_FILE "AudioMediaUdsWriter.cc"

namespace sipxsua {

using TClock = chrono::high_resolution_clock;
using TDuration = chrono::duration<float, micro>;

AudioMediaUdsWriter::AudioMediaUdsWriter(const string &path)
    : AudioMedia(), UdsWriter(path) {
  id == PJSUA_INVALID_ID;
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "AudioMediaUdsWriter", 8192, 8192,
                        NULL);
  CHECK_NOTNULL(pool);
  open();
}

AudioMediaUdsWriter::~AudioMediaUdsWriter() {
  LOG(INFO) << "~dtor";
  if (id != PJSUA_INVALID_ID) {
    unregisterMediaPort();
  }
  if (NULL != src_state) {
    src_state = src_delete(src_state);
    if (src_state) {
      LOG(FATAL) << ": src_delete() failed";
    }
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

void AudioMediaUdsWriter::createRecorder(
    const pj::MediaFormatAudio &audioFormat, unsigned sampleRate,
    unsigned frameTimeMsec, int resampleLevel) {
  CHECK_EQ(PJSUA_INVALID_ID, id);
  // 只支持 Mono channel
  CHECK_EQ(1, audioFormat.channelCount) << "仅支持 mono channel audio";
  // 只支持 16bits sampling
  CHECK_EQ(16, audioFormat.bitsPerSample) << "仅支持 16bits PCM";

  this->frameTimeMsec = frameTimeMsec;
  this->audioFormat = audioFormat;
  this->sampleRate = sampleRate;

  // 远端的声音媒体作为 source，获取它的格式规格
  // 计算缓冲区大小
  size_t bytes_per_sec = audioFormat.clockRate * audioFormat.channelCount *
                         audioFormat.bitsPerSample / 8; // 每秒的字节数
  buffer_size = bytes_per_sec * frameTimeMsec / 1000;
  unsigned int samples_per_frame = audioFormat.clockRate *
                                   audioFormat.channelCount *
                                   audioFormat.frameTimeUsec / 1000000;

  //分配缓冲区
  capture_buffer =
      (uint8_t *)pj_pool_calloc(pool, buffer_size, sizeof(uint8_t));

  // 新建 resampler
  if (sampleRate != audioFormat.clockRate) {
    DVLOG(1) << "create resampler. level=" << resampleLevel;
    int src_errno;
    src_state = src_new(resampleLevel, audioFormat.channelCount, &src_errno);
    CHECK_NOTNULL(src_state);
    CHECK_EQ(0, src_errno)
        << ": src_new() error in AudioMediaUdsWriter::createRecorder(). "
        << src_strerror(src_errno);
    // 重采样率比例：output_sample_rate / input_sample_rate
    src_data.src_ratio = (double)sampleRate / (double)audioFormat.clockRate;
    // 计算输入数据的 frame(采样!) 个数！ (PJ一般是16bit) 的 PCM
    src_data.input_frames = audioFormat.clockRate * frameTimeMsec / 1000;
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
    // 采样数不能为0！
    CHECK_LT(0, src_data.input_frames)
        << ": resample input_frames can not be zero!";
    CHECK_LT(0, src_data.output_frames)
        << ": resample output_frames can not be zero!";
    CHECK_NOTNULL(src_data.data_in);
    CHECK_NOTNULL(src_data.data_out);
    CHECK_NOTNULL(resampled_short_array);
  }

  // 建立内存采集 Audio Port
  DVLOG(1) << "createRecorder() ... " << endl
           << "  path=" << path << ", " << endl
           << "  fd=" << fd << ", " << endl
           << "  frame_time_msec=" << frameTimeMsec << ", " << endl
           << "  buffer_size=" << buffer_size << ", " << endl
           << "  sample_rate=" << audioFormat.clockRate << ", " << endl
           << "  channel=" << audioFormat.channelCount << ", " << endl
           << "  samples_per_frame=" << samples_per_frame << ", " << endl
           << "  bits_per_sample=" << audioFormat.bitsPerSample << ", " << endl
           << "  resample_samplerate=" << sampleRate << ", " << endl
           << "  resample_input_samples=" << src_data.input_frames << ", "
           << endl
           << "  resample_output_samples=" << src_data.output_frames << ", "
           << endl
           << "  resample_ration=" << src_data.src_ratio;
  PJSUA2_CHECK_EXPR(pjmedia_mem_capture_create(
      pool, capture_buffer, buffer_size, audioFormat.clockRate,
      audioFormat.channelCount, samples_per_frame, audioFormat.bitsPerSample, 0,
      &port));
  // 接收回调
  PJSUA2_CHECK_EXPR(
      pjmedia_mem_capture_set_eof_cb2(port, (void *)this, cb_mem_capture_eof));
  // 如果上面一步失败，就不会产生有效的 media id.
  // C++ way： 把 Port 加入到 conf，并接收新的 port id 到这个类的 id 属性
  registerMediaPort2(port, pool);
  LOG(INFO) << "Recorder[" << id << "]==>" << fd << "==>" << path;
}

void AudioMediaUdsWriter::cb_mem_capture_eof(pjmedia_port *port,
                                             void *usr_data) {
  ((AudioMediaUdsWriter *)usr_data)->write();
}

void AudioMediaUdsWriter::write() {
  auto tsBegin = TClock::now();

  // 如果不用 resample，缓冲的数据可以直接发送
  uint8_t *buffer = NULL;
  size_t length = 0;
  // Resample?
  if (NULL != src_state) {
    // 进行重采样
    // 重采样！
    int src_errno;
    // Resampler 原始16bit采样数据转float采样数据装载到输入缓冲
    src_short_to_float_array((const short *)capture_buffer,
                             (float *)src_data.data_in, src_data.input_frames);
    src_data.input_frames_used = 0;
    src_data.output_frames_gen = 0;
    src_data.end_of_input = 0;
    while (src_data.output_frames_gen < src_data.output_frames) {
      src_errno = src_process(src_state, &src_data);
      CHECK_EQ(0, src_errno)
          << ": src_process() error: " << src_strerror(src_errno);
      CHECK_LT(0, src_data.output_frames_gen)
          << ": src_process() generates zero frames.";
    }
    // 将输出float采样的缓冲数据该写成16Kbit采样的数据写入到结果缓冲
    src_float_to_short_array(src_data.data_out, resampled_short_array,
                             src_data.output_frames);
    // 把发送缓冲指向到这里!
    buffer = (uint8_t *)resampled_short_array;
    length = src_data.output_frames * sizeof(short);
  } else {
    // 把发送缓冲指向未经重采样的 buffer!
    buffer = capture_buffer;
    length = buffer_size;
  }

  CHECK_NOTNULL(buffer);
  CHECK_LT(0, length);
  UdsWriter::write((void *)buffer, length);
  TDuration elapsed = TClock::now() - tsBegin;
  VLOG_EVERY_N(3, 100) << "write()"
                       << " " << length << " bytes"
                       << " "
                       << "elapsed = " << elapsed.count() << " usec";
}

} // namespace sipxsua
