#include "AudioMediaUdsReader.hxx"

#include <errno.h>
#include <stdint.h>
#include <unistd.h>

#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <iostream>
#include <sstream>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

#define THIS_FILE "AudioMediaUdsReader.cxx"

using namespace std;
using namespace pj;

namespace sipxsua {

AudioMediaUdsReader::AudioMediaUdsReader() : AudioMedia() {
  id == PJSUA_INVALID_ID;
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "AudioMediaUdsReader", 8192, 8192,
                        NULL);
}

AudioMediaUdsReader::~AudioMediaUdsReader() {
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

void AudioMediaUdsReader::createPlayer(const pj::MediaFormatAudio &audioFormat,
                                       const std::string &path,
                                       unsigned sampleRate,
                                       unsigned bufferMSec) {
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
  bind_addr = (sockaddr_un *)pj_pool_calloc(pool, 1, sizeof(sockaddr_un));
  bind_addr->sun_family = AF_LOCAL;
  strncpy(bind_addr->sun_path, path.c_str(), sizeof(bind_addr->sun_path) - 1);
  sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    ostringstream oss;
    oss << "socket error (" << errno << "): " << strerror(errno);
    cerr << oss.str() << endl;
    throw new runtime_error(oss.str());
  }
  struct stat statbuf;
  if (!stat(path.c_str(), &statbuf)) {
    if (unlink(path.c_str())) {
      ostringstream oss;
      oss << "unlink error (" << errno << "): " << strerror(errno);
      cerr << oss.str() << endl;
      throw new runtime_error(oss.str());
    }
  }
  if (bind(sockfd, (sockaddr *)bind_addr, sizeof(bind_addr))) {
    ostringstream oss;
    oss << "socket bind error (" << errno << "): " << strerror(errno);
    cerr << oss.str() << endl;
    throw new runtime_error(oss.str());
  }

  // 建立内存播放 Audio Port
  pjmedia_mem_player_create(
      pool, buffer, buffer_size, audioFormat.clockRate,
      audioFormat.channelCount, samples_per_frame, audioFormat.bitsPerSample,
      pjmedia_mem_player_option::PJMEDIA_MEM_NO_LOOP, &port);
  // C++ way： 把 Port 加入到 conf，并接收新的 port id 到这个类的 id 属性
  registerMediaPort2(port, pool);
  // 如果上面一步失败，就不会产生有效的 media id.
  // 接收回调
  pjmedia_mem_player_set_eof_cb2(port, (void *)this, cb_mem_play_eof);
}

void AudioMediaUdsReader::onBufferEof() {}

void AudioMediaUdsReader::cb_mem_play_eof(pjmedia_port *port, void *usr_data) {
  ((AudioMediaUdsReader *)usr_data)->onBufferEof();
}

} // namespace sipxsua
