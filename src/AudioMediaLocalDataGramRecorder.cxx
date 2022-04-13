#include "AudioMediaLocalDataGramRecorder.hxx"

#include <errno.h>
#include <error.h>
#include <memory.h>
#include <string.h>

#include <sys/socket.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

AudioMediaLocalDataGramRecorder::AudioMediaLocalDataGramRecorder()
    : pj::AudioMedia() {
  id == PJSUA_INVALID_ID;
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "AudioMediaLocalDataGramRecorder",
                        8192, 8192, NULL);
  err_buf = (char *)pj_pool_calloc(pool, err_sz, sizeof(char));
}

AudioMediaLocalDataGramRecorder::~AudioMediaLocalDataGramRecorder() {
  if (id != PJSUA_INVALID_ID) {
    unregisterMediaPort();
  }
  if (sockfd != 0) {
    close(sockfd);
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

void AudioMediaLocalDataGramRecorder::createRecorder(
    const std::string &sendtoFile, unsigned clockRate, unsigned channelCount,
    unsigned samplesPerFrame, unsigned bitsPerSample) {
  if (id != PJSUA_INVALID_ID) {
    /// TODO: 不允许重复创建！！！
    throw new std::runtime_error("Duplicate invoking on createRecorder");
  }

  sendto_addr = (sockaddr_un *)pj_pool_calloc(pool, 1, sizeof(sockaddr_un));
  sendto_addr->sun_family = AF_LOCAL;
  strncpy(sendto_addr->sun_path, sendtoFile.c_str(),
          sizeof(sendto_addr->sun_path) - 1);

  sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    std::ostringstream oss;
    oss << "socket error (" << errno << "): " << strerror(errno);
    std::cerr << oss.str() << std::endl;
    throw new std::runtime_error(oss.str());
  }

  // 1 秒的缓冲
  buffer_size = clockRate * channelCount * bitsPerSample / 8;
  buffer = (uint8_t *)pj_pool_calloc(pool, buffer_size, sizeof(uint8_t));

  PJ_LOG(4, ("AudioMediaLocalDataGramRecorder",
             "createRecorder: bufferSize=%d, clockRate=%d, channelCount=%d, "
             "samplesPerFrame=%d,bitsPerSample=%d",
             buffer_size, clockRate, channelCount, samplesPerFrame,
             bitsPerSample));

  pjmedia_mem_capture_create(pool, buffer, buffer_size, clockRate, channelCount,
                             samplesPerFrame, bitsPerSample, 0, &port);
  // 高层 C++ 方法：
  // 把 Port 加入到 conf，并接收新的 port id 到这个类的 id 属性
  registerMediaPort2(port, pool);

  if (id == PJSUA_INVALID_ID) {
    std::ostringstream oss;
    oss << "pjsua_conf_add_port PJSUA_INVALID_ID error: " << err_buf;
    std::cerr << oss.str() << std::endl;
    throw new std::runtime_error(oss.str());
  } else {
    PJ_LOG(4, ("AudioMediaLocalDataGramRecorder",
               "createRecorder: conf_port_id=%d", id));
  }

  pjmedia_mem_capture_set_eof_cb2(port, (void *)this, portEofFunc);
}

void AudioMediaLocalDataGramRecorder::portEofFunc(pjmedia_port *port,
                                                  void *usr_data) {
  ((AudioMediaLocalDataGramRecorder *)usr_data)->onBufferFull(port);
}

void AudioMediaLocalDataGramRecorder::onBufferFull(pjmedia_port *port) {
  ssize_t n = sendto(sockfd, buffer, buffer_size, 0,
                     (struct sockaddr *)sendto_addr, sizeof(*sendto_addr));
  if (n < 0) {
    switch (errno) {
    case ENOENT:
      break;
    default: {
      PJ_LOG(2, ("AudioMediaLocalDataGramRecorder",
                 "Unix Socket DATAGRAM send error (%d): %s", errno,
                 strerror(errno)));
    } break;
    }
  }
  memset(buffer, 0, buffer_size);
}
