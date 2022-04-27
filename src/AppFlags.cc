#include "AppFlags.hh"

#include <cstring>

#include "SipFlags.hh"

static bool valide_dst_uri(const char *flagname, const std::string &value);

DEFINE_string(dst_uri, "", "被叫的 SIP URI");
DEFINE_validator(dst_uri, &valide_dst_uri);

bool valide_dst_uri(const char *flagname, const std::string &value) {
  if (FLAGS_list_codecs)
    return true;
  return !value.empty();
}
