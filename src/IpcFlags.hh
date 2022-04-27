#ifndef __sipxsua_IpcFlags_h__
#define __sipxsua_IpcFlags_h__ 1

#include <gflags/gflags.h>

DECLARE_string(aud_playback_path);
DECLARE_uint32(aud_playback_samplerate);
DECLARE_uint32(aud_playback_frametime);
DECLARE_uint32(aud_playback_resample_level);

DECLARE_string(aud_capture_path);
DECLARE_uint32(aud_capture_samplerate);
DECLARE_uint32(aud_capture_frametime);

#endif
