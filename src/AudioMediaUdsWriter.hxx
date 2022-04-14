#ifndef __AudioMediaUdsWriter__
#define __AudioMediaUdsWriter__

#include <stdint.h>

#include <sys/un.h>

#include <pjlib.h>
#include <pjmedia.h>
#include <pjsua2.hpp>

#include <samplerate.h>

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
   * @param call
   * @param sendto_path
   * @param clock_rate
   * @param channel_count
   * @param samples_per_frame
   * @param bits_per_sample
   * @param buffer_msec
   */
  void createRecorder(const pj::Call *call, const std::string &sendto_path,
                      unsigned sample_rate, unsigned buffer_msec = 200);

  const unsigned get_src_clock_rate() { return _src_clock_rate; };
  const unsigned get_src_channel_count() { return _src_channel_count; };
  const unsigned get_src_samples_per_frame() { return _src_samples_per_frame; };
  const unsigned get_src_bits_per_sample() { return _src_bits_per_sample; };

  const unsigned get_dst_clock_rate() { return _dst_clock_rate; };
  // const unsigned get_dst_channel_count() { return _dst_channel_count; };
  // const unsigned get_dst_samples_per_frame() { return _dst_samples_per_frame;
  // }; const unsigned get_dst_bits_per_sample() { return _dst_bits_per_sample;
  // };

protected:
  void onFullfill(pjmedia_port *port);

private:
  char *err_buf;
  size_t err_sz = PJ_LOG_MAX_SIZE;

  int sockfd = -1;
  sockaddr_un *sendto_addr;

  pjmedia_port *port;

  uint8_t *buffer;
  size_t buffer_size;

  static void cb_mem_capture_eof(pjmedia_port *port, void *usr_data);

  pj_caching_pool cachingPool;
  pj_pool_t *pool;

  unsigned _src_clock_rate;
  unsigned _src_channel_count;
  unsigned _src_samples_per_frame;
  unsigned _src_bits_per_sample;

  unsigned _dst_clock_rate;
  // unsigned _dst_channel_count;
  // unsigned _dst_samples_per_frame;
  // unsigned _dst_bits_per_sample;

  SRC_STATE *src_state = NULL;
  SRC_DATA src_data = {0};
  short *resampled_short_array = NULL;
};

} // namespace sipxsua

#endif
