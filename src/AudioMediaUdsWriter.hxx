#ifndef __AudioMediaUdsWriter__
#define __AudioMediaUdsWriter__

#include <stdint.h>

#include <sys/un.h>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

namespace sipxsua {

/**
 * Audio Media Recorder, send recoreded data to Unix Socket.
 */
class AudioMediaUdsWriter : public pj::AudioMedia {
public:
  /**
   * @brief Construct a new Audio Media Unix Dgram Recorder object
   *
   */
  AudioMediaUdsWriter();

  /**
   * @brief Destroy the Audio Media Unix Dgram Recorder object
   *
   */
  ~AudioMediaUdsWriter();

  /**
   * @brief Create a Recorder object
   *
   * @param sendtoFile
   * @param clockRate
   * @param channelCount
   * @param samplesPerFrame
   * @param bitsPerSample
   */
  void createRecorder(const std::string &sendtoFile, unsigned clockRate,
                      unsigned channelCount, unsigned samplesPerFrame,
                      unsigned bitsPerSample);

protected:
  void onBufferFull(pjmedia_port *port);

private:
  char *err_buf;
  size_t err_sz = PJ_LOG_MAX_SIZE;

  int sockfd = -1;
  sockaddr_un *sendto_addr;

  pjmedia_port *port;

  uint8_t *buffer;
  size_t buffer_size;

  static void portEofFunc(pjmedia_port *port, void *usr_data);

  pj_caching_pool cachingPool;
  pj_pool_t *pool;
};

} // namespace sipxsua

#endif
