#include "IpcFlags.hh"

#include <cstdint>
#include <cstring>

#include <samplerate.h>

#include "SipFlags.hh"

static bool valid_path(const char *flagname, const std::string &value);

DEFINE_string(aud_playback_path, "", "向该文件放音(Signed 16-bit PCM Mono)");
DEFINE_validator(aud_playback_path, &valid_path);
DEFINE_uint32(aud_playback_samplerate, 48000, "放音的采样率");
DEFINE_uint32(aud_playback_frametime, 20, "放音的frame时长(milli)");
DEFINE_uint32(
    aud_playback_resample_level, SRC_SINC_MEDIUM_QUALITY,
    "放音的重采样(如有必要)级别(0=best/slowest ... 4=poorest/fastest)");
DEFINE_validator(aud_playback_resample_level, [](const char *flagname,
                                                 uint32_t value) {
  return ((value >= SRC_SINC_BEST_QUALITY) && (value <= SRC_LINEAR));
});

DEFINE_string(aud_capture_path, "", "从该文件拾音(Signed 16-bit PCM Mono)");
DEFINE_validator(aud_capture_path,
                 [](const char *flagname, const std::string &value) {
                   if (FLAGS_list_codecs)
                     return true;
                   return !value.empty();
                 });

DEFINE_uint32(aud_capture_samplerate, 48000, "拾音的采样率");
DEFINE_uint32(aud_capture_frametime, 20, "拾音的frame时长(milli)");

DEFINE_string(event_fifo, "", "向这个 FIFO 发送事件通知");

bool valid_path(const char *flagname, const std::string &value) {
  if (FLAGS_list_codecs)
    return true;
  return !value.empty();
}
