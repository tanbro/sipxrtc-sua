#ifndef __sipxsua_AudioMediaUdsReader__
#define __sipxsua_AudioMediaUdsReader__ 1

#include <sys/un.h>

#include <cstdint>
#include <mutex>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

#include "UdsReader.hh"

namespace sipxsua {

/**
 * Audio Media player, read received audio data from Unix Socket.
 */
class AudioMediaUdsReader : public pj::AudioMedia, public UdsReader {
public:
  AudioMediaUdsReader(const std::string &path);
  virtual ~AudioMediaUdsReader();

  void createPlayer(const pj::MediaFormatAudio &audioFormat);

  void read();

protected:
  /**
   * @brief 源头音频格式信息
   */
  pj::MediaFormatAudio audioFormat;

  void onBufferEof();

private:
  static std::map<int, AudioMediaUdsReader *> instances;

  pjmedia_port *port = NULL;

  std::mutex buffer_mtx;
  size_t buffer_size;
  uint8_t *play_buffer = NULL;
  uint8_t *read_buffer = NULL;
  void *read_buffer0 = NULL;

  static void cb_mem_play_eof(pjmedia_port *port, void *usr_data);

  pj_caching_pool cachingPool;
  pj_pool_t *pool;
};

} // namespace sipxsua

#endif
