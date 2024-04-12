#ifndef __sipxsua_SuaLogWriter_h__
#define __sipxsua_SuaLogWriter_h__ 1

#include <pjsua2.hpp>

namespace sipxsua {

class SuaLogWriter : public pj::LogWriter {
public:
  /// @brief
  /// pjproject/pjlib/include/pj/config.h:
  /// PJ_LOG_MAX_LEVEL:
  /// logging level/verbosity.
  /// Lower number indicates higher importance, with the highest importance has level zero.
  /// The least important level is five in this implementation, but this can be extended by supplying the appropriate implementation.
  /// The level conventions:
  ///  - 0: fatal error
  ///  - 1: error
  ///  - 2: warning
  ///  - 3: info
  ///  - 4: debug
  ///  - 5: trace
  ///  - 6: more detailed trace
  /// Default: 4
  virtual void write(const pj::LogEntry &entry) override;
};

} // namespace sipxsua

#endif
