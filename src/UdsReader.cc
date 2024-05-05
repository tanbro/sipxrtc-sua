#include "UdsReader.hh"

#include <sys/socket.h>
#include <sys/stat.h>

#include <cstring>
#include <thread>

#include <glog/logging.h>

namespace sipxsua {

using namespace std;

UdsReader::UdsReader(const std::string &path) : path(path) {
  memset(&addr, 0, sizeof(addr));
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, path.c_str(), sizeof(addr.sun_path) - 1);
}

UdsReader::~UdsReader() {
  if (!(fd < 0)) {
    close();
  }
}

int UdsReader::open() {
  UdsBase::open();
  struct stat statbuf;
  if (!stat(addr.sun_path, &statbuf)) {
    LOG(WARNING) << "unlink " << path;
    CHECK_ERR(unlink(addr.sun_path));
  }
  CHECK_ERR(bind(fd, (const sockaddr *)&addr, sizeof(addr)));
  LOG(INFO) << "bind " << fd << ":" << path;
  return fd;
}

void UdsReader::close() {
  UdsBase::close();
  struct stat statbuf;
  if (!stat(addr.sun_path, &statbuf)) {
    LOG(INFO) << "unlink " << path;
    CHECK_ERR(unlink(addr.sun_path));
  }
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
