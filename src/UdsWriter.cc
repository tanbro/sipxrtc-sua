#include "UdsWriter.hh"

#include <sys/socket.h>

#include <cstring>

#include <glog/logging.h>

using namespace std;

namespace sipxsua {

UdsWriter::UdsWriter(const std::string &path) : path(path) {
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
}

ssize_t UdsWriter::write(void *data, size_t length) {
  CHECK_LT(0, fd);
  return sendto(fd, data, length, 0, (struct sockaddr *)&addr, sizeof(addr));
}

} // namespace sipxsua
