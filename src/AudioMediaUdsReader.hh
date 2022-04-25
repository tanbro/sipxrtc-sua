#ifndef __sipxsua_AudioMediaUdsReader__
#define __sipxsua_AudioMediaUdsReader__ 1

#include <sys/un.h>

#include <cstdint>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

namespace sipxsua {

/**
 * Audio Media player, read received audio data from Unix Socket.
 */
class AudioMediaUdsReader : public pj::AudioMedia {
public:
  AudioMediaUdsReader();
  virtual ~AudioMediaUdsReader();

  void createPlayer(const pj::MediaFormatAudio &audioFormat,
                    const std::string &path, unsigned sampleRate,
                    unsigned bufferMSec = 100);

  int getFd() { return sockfd; };

  void runOnce();

protected:
  /**
   * @brief 源头音频格式信息
   */
  pj::MediaFormatAudio audioFormat;
  /**
   * @brief 目标音频采样率
   */
  unsigned sampleRate;
  void onBufferEof();

private:
  int sockfd = -1;
  sockaddr_un recv_addr;

  pjmedia_port *port;

  uint8_t *buffer;
  size_t buffer_size;

  uint8_t recv_buffer[1920];

  static void cb_mem_play_eof(pjmedia_port *port, void *usr_data);

  pj_caching_pool cachingPool;
  pj_pool_t *pool;
};

} // namespace sipxsua

#endif
