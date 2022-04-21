#ifndef __sipxsua_SuaLogWriter_h__
#define __sipxsua_SuaLogWriter_h__ 1

#include <pjsua2.hpp>

namespace sipxsua {

class SuaLogWriter : public pj::LogWriter {
public:
  void write(const pj::LogEntry &entry);
};

} // namespace sipxsua

#endif