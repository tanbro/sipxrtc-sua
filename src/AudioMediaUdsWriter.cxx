#include "AudioMediaUdsWriter.hxx"

#include <errno.h>
#include <error.h>
#include <memory.h>
#include <string.h>

#include <sys/socket.h>

#include <iostream>
#include <sstream>
#include <stdexcept>

namespace sipxsua {

AudioMediaUdsWriter::AudioMediaUdsWriter() : pj::AudioMedia() {
  id == PJSUA_INVALID_ID;
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "AudioMediaUdsWriter", 8192, 8192,
                        NULL);
  err_buf = (char *)pj_pool_calloc(pool, err_sz, sizeof(char));
}

AudioMediaUdsWriter::~AudioMediaUdsWriter() {
  if (id != PJSUA_INVALID_ID) {
    unregisterMediaPort();
  }
  if (sockfd != 0) {
    close(sockfd);
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

void AudioMediaUdsWriter::createRecorder(
    const pj::Call *call, const std::string &sendto_path, unsigned clock_rate,
    unsigned channel_count, unsigned samples_per_frame,
    unsigned bits_per_sample, unsigned buffer_msec) {
  if (id != PJSUA_INVALID_ID) {
    /// TODO: 不允许重复创建！！！
    throw new std::runtime_error("Duplicate invoking on createRecorder");
  }

  // 打开 Unix domain socket
  sendto_addr = (sockaddr_un *)pj_pool_calloc(pool, 1, sizeof(sockaddr_un));
  sendto_addr->sun_family = AF_LOCAL;
  strncpy(sendto_addr->sun_path, sendto_path.c_str(),
          sizeof(sendto_addr->sun_path) - 1);

  sockfd = socket(AF_LOCAL, SOCK_DGRAM, 0);
  if (sockfd == -1) {
    std::ostringstream oss;
    oss << "socket error (" << errno << "): " << strerror(errno);
    std::cerr << oss.str() << std::endl;
    throw new std::runtime_error(oss.str());
  }

  // 远端的声音媒体作为 source，获取它的格式规格
  auto med = call->getAudioMedia(-1);
  auto port_info = med.getPortInfo();
  pjsua_conf_port_info _port_info;
  pjsua_conf_get_port_info(port_info.portId, &_port_info);

  // 缓冲远端过来的声音流
  // 计算缓冲区大小
  // 每秒的字节数
  size_t bytes_per_sec = _port_info.clock_rate * _port_info.channel_count *
                         _port_info.bits_per_sample / 8;
  buffer_size = bytes_per_sec * buffer_msec / 1000;
  buffer = (uint8_t *)pj_pool_calloc(pool, buffer_size, sizeof(uint8_t));

  // 建立内存捕获 Audio Port
  pjmedia_mem_capture_create(pool, buffer, buffer_size, _port_info.clock_rate,
                             _port_info.channel_count,
                             _port_info.samples_per_frame,
                             _port_info.bits_per_sample, 0, &port);
  // C++ way： 把 Port 加入到 conf，并接收新的 port id 到这个类的 id 属性
  registerMediaPort2(port, pool);
  // 如果上面一步失败，就不会产生有效的 media id.
  if (id == PJSUA_INVALID_ID) {
    std::ostringstream oss;
    oss << "pjsua_conf_add_port PJSUA_INVALID_ID error: " << err_buf;
    std::cerr << oss.str() << std::endl;
    throw new std::runtime_error(oss.str());
  } else {
    PJ_LOG(4, ("AudioMediaUdsWriter", "createRecorder: conf_port_id=%d", id));
  }
  // 接收回调
  pjmedia_mem_capture_set_eof_cb2(port, (void *)this, cb_mem_capture_eof);
}

void AudioMediaUdsWriter::cb_mem_capture_eof(pjmedia_port *port,
                                             void *usr_data) {
  ((AudioMediaUdsWriter *)usr_data)->onFullfill(port);
}

void AudioMediaUdsWriter::onFullfill(pjmedia_port *port) {
  ssize_t n = sendto(sockfd, buffer, buffer_size, 0,
                     (struct sockaddr *)sendto_addr, sizeof(*sendto_addr));
  if (n < 0) {
    switch (errno) {
    case ENOENT:
      break;
    default: {
      PJ_LOG(2,
             ("AudioMediaUdsWriter", "Unix Socket DATAGRAM send error (%d): %s",
              errno, strerror(errno)));
    } break;
    }
  }
  memset(buffer, 0, buffer_size);
}

} // namespace sipxsua