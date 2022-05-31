#ifndef __sipxsua_SipFlags_h__
#define __sipxsua_SipFlags_h__ 1

#include <gflags/gflags.h>

DECLARE_string(sip_public_address);
DECLARE_uint32(sip_port);
DECLARE_uint32(sip_port_range);

DECLARE_uint32(rtp_port);
DECLARE_uint32(rtp_port_range);
DECLARE_int32(srtp_use);

DECLARE_bool(list_codecs);

DECLARE_int32(sip_console_level);
DECLARE_int32(sip_log_level);
DECLARE_string(sip_log_file);

#endif
