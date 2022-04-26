#include "AudioMediaUdsReader.hh"

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <chrono>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

#include <glog/logging.h>

#define THIS_FILE "AudioMediaUdsReader.cxx"

using namespace std;
using namespace pj;

namespace sipxsua {

using TClock = chrono::high_resolution_clock;
using TDuration = chrono::duration<float, micro>;

AudioMediaUdsReader::AudioMediaUdsReader(const string &path)
    : AudioMedia(), UdsReader(path) {
  id == PJSUA_INVALID_ID;
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "AudioMediaUdsReader", 8192, 8192,
                        NULL);
  CHECK_NOTNULL(pool);
  //
  open();
  DLOG(INFO) << "activated! fd=" << fd;
}

AudioMediaUdsReader::~AudioMediaUdsReader() {
  /// TODO: 死循环，退不出来的
  if (id != PJSUA_INVALID_ID) {
    unregisterMediaPort();
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

void AudioMediaUdsReader::createPlayer(const pj::MediaFormatAudio &audioFormat,
                                       unsigned sampleRate,
                                       unsigned bufferMSec) {
  CHECK_EQ(id, PJSUA_INVALID_ID) << ":不允许重复创建 player";

  // 只支持 Mono channel
  CHECK_EQ(1, audioFormat.channelCount) << "仅支持 mono audio";
  // 只支持 16bits sampling
  CHECK_EQ(16, audioFormat.bitsPerSample) << "仅支持 16bits sampling";

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
  play_buffer = (uint8_t *)pj_pool_calloc(pool, buffer_size, sizeof(uint8_t));
  read_buffer = (uint8_t *)pj_pool_calloc(pool, buffer_size, sizeof(uint8_t));
  read_buffer0 = (uint8_t *)pj_pool_calloc(pool, buffer_size, sizeof(uint8_t));

  // 建立内存播放 Audio Port
  DVLOG(1) << "createPlayer() ... " << endl
           << "  path=" << path << ", " << endl
           << "  fd=" << fd << ", " << endl
           << "  buffer_msec=" << bufferMSec << ", " << endl
           << "  buffer_size=" << buffer_size << ", " << endl
           << "  sample_rate=" << audioFormat.clockRate << ", " << endl
           << "  channel=" << audioFormat.channelCount << ", " << endl
           << "  samples_per_frame=" << samples_per_frame << ", " << endl
           << "  bits_per_sample=" << audioFormat.bitsPerSample;
  PJSUA2_CHECK_EXPR(pjmedia_mem_player_create(
      pool, play_buffer, buffer_size, audioFormat.clockRate,
      audioFormat.channelCount, samples_per_frame, audioFormat.bitsPerSample, 0,
      &port));
  // 接收回调
  PJSUA2_CHECK_EXPR(
      pjmedia_mem_player_set_eof_cb2(port, (void *)this, cb_mem_play_eof));
  // 如果上面一步失败，就不会产生有效的 media id.
  // C++ way： 把 Port 加入到 conf，并接收新的 port id 到这个类的 id 属性
  registerMediaPort2(port, pool);
  DVLOG(1) << "createPlayer() ... id=" << id;
}

void AudioMediaUdsReader::onBufferEof() {
  lock_guard<mutex> lk(buffer_mtx);
  memcpy(play_buffer, read_buffer, buffer_size);
  memset(read_buffer, 0, buffer_size);
}

void AudioMediaUdsReader::read() {
  auto tsBegin = TClock::now();
  CHECK_ERR(UdsReader::read(read_buffer0, buffer_size));
  {
    lock_guard<mutex> lk(buffer_mtx);
    memcpy(read_buffer, read_buffer0, buffer_size);
  }
  TDuration elapsed = TClock::now() - tsBegin;
  VLOG_EVERY_N(3, 100) << "read()"
                       << " "
                       << "elapsed = " << elapsed.count() << " usec";
}

void AudioMediaUdsReader::cb_mem_play_eof(pjmedia_port *port, void *usr_data) {
  ((AudioMediaUdsReader *)usr_data)->onBufferEof();
}

} // namespace sipxsua
