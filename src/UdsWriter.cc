#include "UdsWriter.hh"

#include <sys/socket.h>

#include <cstring>
#include <iomanip>
#include <thread>

#include <glog/logging.h>

namespace sipxsua {

using namespace std;

UdsWriter::UdsWriter(const std::string &path) : path(path) {
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
}

ssize_t UdsWriter::write(void *data, size_t length) {
  CHECK_LT(0, fd);
  VLOG(6) << "[" << hex << this_thread::get_id() << "] "
          << ">>> sendto("
          << "fd=" << hex << fd << dec << ", "
          << "data=" << data << ", "
          << "length=" << length << ", "
          << "addr=\"" << path << "\""
          << ")";
  ssize_t res =
      sendto(fd, data, length, 0, (struct sockaddr *)&addr, sizeof(addr));
  if (res < 0) {
    switch (errno) {
    case ENOENT:
      break;
    case ECONNREFUSED:
      break;
    default:
      PCHECK(errno) << ": sendto() error: ";
      break;
    }
  }
  VLOG(6) << "[" << hex << this_thread::get_id() << "] "
          << "<<< sendto("
          << "fd=" << hex << fd << dec << ", "
          << "data=" << data << ", "
          << "length=" << length << ", "
          << "addr=\"" << path << "\""
          << ")"
          << " -> " << res;
  return res;
}

} // namespace sipxsua
