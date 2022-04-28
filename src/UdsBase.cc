#include "UdsBase.hh"

#include <sys/socket.h>
#include <sys/un.h>

#include <glog/logging.h>

namespace sipxsua {

UdsBase::~UdsBase() {
  if (!(fd < 0)) {
    close();
  }
}

int UdsBase::open() {
  CHECK_GT(0, fd); // 不许重复打开
  CHECK_ERR(fd = socket(AF_UNIX, SOCK_DGRAM, 0));
  return fd;
}

void UdsBase::close() {
  CHECK_LT(0, fd); // 不许重复关闭
  CHECK_ERR(::close(fd));
  fd = -1;
}

int UdsBase::getFd() { return fd; }

} // namespace sipxsua