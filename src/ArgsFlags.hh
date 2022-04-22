#ifndef __sipxsua_ArgsFlags_h__
#define __sipxsua_ArgsFlags_h__ 1

#include <stdint.h>

#include <iostream>

#include <gflags/gflags.h>
#include <pjsua2.hpp>

static bool valide_pj_srtp_use(const char *flagname, int32_t value);

DEFINE_bool(list_pj_codecs, false,
            "在 PJ 模块初始化后, 列出其支持的音频编码, 然后退出程序");

DEFINE_int32(pj_log_console_level, -1,
             "设置 PJ 的 Console 输出级别. `<0` 表示采用默认值 (6=very "
             "detailed..1=error only, 0=disabled)");
DEFINE_int32(pj_log_level, -1,
             "设置 PJ 的日志级别. `<0` 表示采用默认值 (6=very "
             "detailed..1=error only, 0=disabled)");
DEFINE_string(pj_log_file, "", "设置 PJ 的日志输出文件. 默认不写文件");

DEFINE_uint32(pj_sip_port, 0,
              "设置 PJ 用于 SIP 信令传输的 UDP 端口。`0` 表示任意可用端口。");
DEFINE_uint32(pj_sip_port_range, 0,
              "设置 PJ 用于 SIP 信令传输的 UDP 端口范围正偏移量。仅当 "
              "`pj_sip_port` 不为 `0` 时有效。");

DEFINE_uint32(pj_rtp_port, 0,
              "设置 PJ 用于 RTP 媒体传输的 UDP 端口。`0` 表示任意可用端口。");
DEFINE_uint32(pj_rtp_port_range, 0,
              "设置 PJ 用于 RTP 媒体传输的 UDP 端口范围正偏移量。仅当 "
              "`pj_rtp_port` 不为 `0` 时有效。");
DEFINE_int32(pj_srtp_use, PJSUA_DEFAULT_USE_SRTP,
             "设置 PJ 的 `srtp` 使用方式。(0:禁用 1:可选 2:必须)");
DEFINE_validator(pj_srtp_use, &valide_pj_srtp_use);

bool valide_pj_srtp_use(const char *flagname, int32_t value) {
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

#endif
