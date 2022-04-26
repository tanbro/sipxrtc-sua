#include "Poller.hh"

#include <chrono>
#include <cstring>
#include <thread>

#include <glog/logging.h>

#include "SipXCall.hh"

using namespace std;

namespace sipxsua {

int Poller::refillFds() {
  lock_guard<mutex> lk(SipXCall::instancesMutex);
  memset(fds, 0, sizeof(fds));
  nfds = 0;
  int i = 0;
  short events = POLLIN;
  auto iterable = SipXCall::instances;
  for (auto it = iterable.begin(); it != iterable.end(); ++it) {
    auto call = *it;
    auto reader = call->getReader();
    if (reader) {
      int fd = reader->getFd();
      if (fd >= 0) {
        fds[i].fd = fd;
        fds[i].events = events;
        ++i;
      }
    }
  }
  nfds = i;
  return i;
}

size_t Poller::performOnce(int timeout) {
  int rc;
  VLOG(1) << "poll=() ...";
  CHECK_ERR(rc = poll(fds, nfds, timeout));

  if (!rc) {
    // Timeout!
    return 0;
  }
  VLOG(1) << "poll() -> " << rc;

  size_t _cnt = 0;
  for (int i = 0; i < rc; ++i) {
    pollfd pfd = fds[i];
    VLOG(1) << pfd.fd << ":" << pfd.revents;
    if (pfd.revents & POLLERR) {
      LOG(WARNING) << "POLLERR";
    }
    if (pfd.revents & POLLHUP) {
      LOG(WARNING) << "POLLHUP";
    }
    if (pfd.revents & POLLNVAL) {
      LOG(WARNING) << "POLLNVAL";
    }
    if (pfd.revents & POLLIN) {
      lock_guard<mutex> lk(SipXCall::instancesMutex);
      auto reader = SipXCall::findReader(pfd.fd);
      if (reader) {
        reader->read();
        ++_cnt;
      }
    }
  }
  return _cnt;
}

void Poller::runForever() {
  while (true) {
    refillFds();
    if (nfds > 0) {
      performOnce(1000);
    } else {
      this_thread::sleep_for(chrono::milliseconds(100));
    }
  }
}
} // namespace sipxsua
