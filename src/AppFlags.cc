#include "AppFlags.hh"

#include <cstring>

#include "SipFlags.hh"

static bool valide_dst_uri(const char *flagname, const std::string &value);

DEFINE_string(dst_uri, "", "被叫的 SIP URI");
DEFINE_validator(dst_uri, &valide_dst_uri);

DEFINE_uint32(max_alive, 1800, "该程序的最大允许生存时长(秒)");

bool valide_dst_uri(const char *flagname, const std::string &value) {
  if (FLAGS_list_codecs)
    return true;
  return !value.empty();
}
