#include "Poller.hh"

#include <cerrno>
#include <chrono>
#include <cstring>
#include <thread>

#include <glog/logging.h>

#include "SipXCall.hh"

namespace sipxsua {

using namespace std;

using TClock = chrono::high_resolution_clock;
using TDuration = chrono::duration<float, milli>;

int Poller::refillFds() {
  CHECK_NE(nullptr, SipXCall::instance);

  memset(fds, 0, sizeof(fds));
  nfds = 0;
  int i = 0;
  short events = POLLIN;

  auto reader = SipXCall::instance->getReader();
  if (reader) {
    auto fd = reader->getFd();
    CHECK_NE(0, fd);
    fds[0].fd = fd;
    fds[0].events = events;
    nfds += 1;
  }

  return nfds;
}

void Poller::runUntil(int timeout, int interval, function<bool()> const &pred) {
  int rc;
  auto tp = TClock::now();
  while (1) {
    refillFds();
    if (nfds) {
      rc = poll(fds, nfds, timeout);
      if (rc < 0) {
        // ERROR, but except "Interrupted system call [4]"
        if (errno == EINTR) {
          LOG(ERROR) << strerror(errno) << " [" << errno << "]";
        } else {
          CHECK_ERR(rc);
        }
      }
      if (rc) {
        tp = TClock::now();
        for (int i = 0; i < rc; ++i) {
          pollfd pfd = fds[i];
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
            auto reader = SipXCall::instance->getReader();
            if (reader) {
              if (reader->getFd()) {
                reader->read();
              }
            }
          }
        } /// endfor
      } else {
        // poll timeout!
        this_thread::sleep_for(chrono::milliseconds(interval));
      }
    } else {
      // no fds!
      this_thread::sleep_for(chrono::milliseconds(interval));
    }
    // 检查：是否继续循环?
    if (!nfds || !rc ||
        (((TDuration)(TClock::now() - tp)).count() > interval)) {
      if (!pred()) {
        break;
      }
    }
  } /// endwhile
}

} // namespace sipxsua
