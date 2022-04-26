#include "UdsReader.hh"

#include <sys/socket.h>
#include <sys/stat.h>

#include <cstring>
#include <thread>

#include <glog/logging.h>

using namespace std;

namespace sipxsua {

int UdsReader::activate() {
  UdsBase::activate();
  struct stat statbuf;
  if (!stat(addr.sun_path, &statbuf)) {
    LOG(WARNING) << "unlink " << path;
    CHECK_ERR(unlink(addr.sun_path));
  }
  CHECK_ERR(bind(fd, (const sockaddr *)&addr, sizeof(addr)));
  return fd;
}

void UdsReader::deactivate() {
  UdsBase::deactivate();
  struct stat statbuf;
  if (!stat(addr.sun_path, &statbuf)) {
    CHECK_ERR(unlink(addr.sun_path));
  }
}

UdsReader::UdsReader(const std::string &path) : path(path) {
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
}

ssize_t UdsReader::read(void *data, size_t length) {
  CHECK_LT(0, fd);
  VLOG(6) << "[" << hex << this_thread::get_id() << "] "
          << ">>> recv("
          << "fd=" << hex << fd << dec << ", "
          << "data=" << data << ", "
          << "length=" << length << ")";
  ssize_t res = recv(fd, data, length, 0);
  VLOG(6) << "[" << hex << this_thread::get_id() << "] "
          << "<<< recv("
          << "fd=" << hex << fd << dec << ", "
          << "data=" << data << ", "
          << "length=" << length << ")"
          << " -> " << dec << res;
  return res;
}

} // namespace sipxsua
