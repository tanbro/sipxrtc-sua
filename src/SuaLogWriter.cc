#include "SuaLogWriter.hh"

#include <glog/logging.h>

namespace sipxsua {

inline void SuaLogWriter::write(const pj::LogEntry &entry) {
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