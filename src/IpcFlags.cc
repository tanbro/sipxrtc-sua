#include "IpcFlags.hh"

#include <cstdint>
#include <cstring>

#include <samplerate.h>

#include "SipFlags.hh"

static bool valide_path(const char *flagname, const std::string &value);
static bool valide_resample_level(const char *flagname, uint32_t value);

DEFINE_string(aud_playback_path, "", "向该文件放声(Signed 16-bit PCM Mono)");
DEFINE_validator(aud_playback_path, &valide_path);
DEFINE_uint32(aud_playback_samplerate, 48000, "放声的采样率");
DEFINE_uint32(aud_playback_frametime, 20, "放声的frame时长(milli)");
DEFINE_uint32(
    aud_playback_resample_level, SRC_SINC_FASTEST,
    "放声的重采样(如有必要)级别(0=best/slowest ... 4=poorest/fastest)");
DEFINE_validator(aud_playback_resample_level, &valide_resample_level);

DEFINE_string(aud_capture_path, "", "从该文件拾声(Signed 16-bit PCM Mono)");
DEFINE_validator(aud_capture_path, &valide_path);
DEFINE_uint32(aud_capture_samplerate, 48000, "拾声的采样率");
DEFINE_uint32(aud_capture_frametime, 20, "拾声的frame时长(milli)");

bool valide_path(const char *flagname, const std::string &value) {
  if (FLAGS_list_codecs)
    return true;
  return !value.empty();
}

bool valide_resample_level(const char *flagname, uint32_t value) {
  return ((value >= SRC_SINC_BEST_QUALITY) && (value <= SRC_LINEAR));
}