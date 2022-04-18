#ifndef __AudioMediaUdsReader__
#define __AudioMediaUdsReader__

#include <stdint.h>

#include <sys/un.h>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

#include <samplerate.h>

namespace sipxsua {

/**
 * Audio Media player, read received audio data from Unix Socket.
 */
class AudioMediaUdsReader : public pj::AudioMedia {
public:
  AudioMediaUdsReader();
  ~AudioMediaUdsReader();

  void createPlayer(const pj::MediaFormatAudio &audioFormat,
                      const std::string &path, unsigned sampleRate,
                      unsigned bufferMSec = 100);

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
  sockaddr_un *bind_addr;

  pjmedia_port *port;

  uint8_t *buffer;
  size_t buffer_size;

  static void cb_mem_play_eof(pjmedia_port *port, void *usr_data);

  pj_caching_pool cachingPool;
  pj_pool_t *pool;

  SRC_STATE *src_state = NULL;
  SRC_DATA src_data = {0};
  short *resampled_short_array = NULL;
};

} // namespace sipxsua

#endif
