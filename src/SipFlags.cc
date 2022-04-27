#include "SipFlags.hh"

#include <cstdint>

#include <pjsua2.hpp>

static bool valide_srtp_use(const char *flagname, int32_t value);

DEFINE_uint32(sip_port, 0,
              "用于 SIP 信令传输的 UDP 端口。`0` 表示任意可用端口。");
DEFINE_uint32(sip_port_range, 0,
              "用于 SIP 信令传输的 UDP 端口可选范围数量。仅当 `sip_port` 不为 "
              "`0` 时有效。");

DEFINE_uint32(rtp_port, 0,
              "用于 RTP 媒体传输的 UDP 端口。`0` 表示任意可用端口。");
DEFINE_uint32(rtp_port_range, 0,
              "用于 RTP 媒体传输的 UDP 端口可选范围数量。仅当`rtp_port` 不为 "
              "`0` 时有效。");
DEFINE_int32(srtp_use, PJSUA_DEFAULT_USE_SRTP,
             "`srtp` 使用方式。(0:禁用 1:可选 2:必须)");
DEFINE_validator(srtp_use, &valide_srtp_use);

DEFINE_bool(list_codecs, false,
            "在 SIP 模块初始化后, 列出其支持的音频编码, 然后退出程序");

DEFINE_int32(sip_console_level, -1,
             "SIP 模块的 Console 输出级别. `<0` 表示采用默认值 (6=very "
             "detailed..1=error only, 0=disabled)");
DEFINE_int32(sip_log_level, -1,
             "设置 PJ 的日志级别. `<0` 表示采用默认值 (6=very "
             "detailed..1=error only, 0=disabled)");
DEFINE_string(sip_log_file, "", "设置 PJ 的日志输出文件. 默认不写文件");

bool valide_srtp_use(const char *flagname, int32_t value) {
  switch (value) {
  case PJMEDIA_SRTP_DISABLED:
    return true;
    break;
  case PJMEDIA_SRTP_OPTIONAL:
    return true;
    break;
  case PJMEDIA_SRTP_MANDATORY:
    return true;
    break;
  default:
    return false;
    break;
  }
}
