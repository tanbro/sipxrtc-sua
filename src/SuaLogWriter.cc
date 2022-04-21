#include "SuaLogWriter.hh"

#include <glog/logging.h>

/// ref: https://www.pjsip.org/pjlib/docs/html/group__PJ__LOG.htm
// level:	The logging verbosity level. Lower number indicates higher
// importance, with level zero indicates fatal error. Only numeral argument is
// permitted (e.g. not variable).
// However, the maximum level of verbosity can not exceed compile time value of
// PJ_LOG_MAX_LEVEL.

namespace sipxsua {

/**
 * pjproject/pjlib/include/pj/config.h:
 *
 * PJ_LOG_MAX_LEVEL:
 *
 * logging level/verbosity. Lower number indicates higher
 * importance, with the highest importance has level zero. The least
 * important level is five in this implementation, but this can be extended
 * by supplying the appropriate implementation.
 *
 * The level conventions:
 *  - 0: fatal error
 *  - 1: error
 *  - 2: warning
 *  - 3: info
 *  - 4: debug
 *  - 5: trace
 *  - 6: more detailed trace
 *
 * Default: 4
 */
void SuaLogWriter::write(const pj::LogEntry &entry) {
  switch (entry.level) {
  case 0: {
    LOG(FATAL) << "[" << entry.threadName << "] " << entry.msg;
  } break;
  case 1: {
    LOG(ERROR) << "[" << entry.threadName << "] " << entry.msg;
  } break;
  case 2: {
    LOG(WARNING) << "[" << entry.threadName << "] " << entry.msg;
  } break;
  case 3: {
    LOG(INFO) << "[" << entry.threadName << "] " << entry.msg;
  } break;
  default: {
    // pj 的 debug, trace ，算作 verbose
    int verboselevel = entry.level - 3;
    VLOG(verboselevel) << "[" << entry.threadName << "] " << entry.msg;
  } break;
  }
}

} // namespace sipxsua