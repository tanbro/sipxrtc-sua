#ifndef __sipxsua_AudioMediaUdsWriter__
#define __sipxsua_AudioMediaUdsWriter__ 1

#include <sys/un.h>

#include <chrono>
#include <cstdint>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

#include <samplerate.h>

#include "UdsWriter.hh"

namespace sipxsua {

/**
 * Audio Media Recorder, send recoreded data to Unix Socket.
 */
class AudioMediaUdsWriter : public pj::AudioMedia, public UdsWriter {
public:
  AudioMediaUdsWriter(const std::string &path);

  virtual ~AudioMediaUdsWriter();

  /**
   * @brief Create a Recorder object
   *
   * @param path 抓取远端的音频媒体，然后写音频数据到这个文件
   * @param audioFormat 远端的来源音频媒体格式
   * @param sampleRate
   * 写出去的目标音频数据的采样率。如果和来源的不同，就会重采样
   * @param frameTimeMsec 远端音频每个DGram的毫秒长度
   */
  void createRecorder(const pj::MediaFormatAudio &audioFormat,
                      unsigned sampleRate, unsigned frameTimeMsec,
                      int resampleLevel);

  /**
   * @brief
   *
   * 这是 UDS 的 DGRAM，所以不要经过 poll 调度，就直接现场写入了！
   */
  void write();

protected:
  /**
   * @brief 源头音频格式信息
   */
  pj::MediaFormatAudio audioFormat;
  /**
   * @brief 目标音频采样率
   */
  unsigned sampleRate;

  unsigned frameTimeMsec;

private:
  pjmedia_port *port = NULL;

  size_t buffer_size = 0;
  uint8_t *capture_buffer = NULL;

  static void cb_mem_capture_eof(pjmedia_port *port, void *usr_data);

  pj_caching_pool cachingPool;
  pj_pool_t *pool;

  SRC_STATE *src_state = NULL;
  SRC_DATA src_data;
  short *resampled_short_array = NULL;
};

} // namespace sipxsua

#endif
