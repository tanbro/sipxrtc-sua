#include "Poller.hh"

#include <chrono>
#include <cstring>
#include <thread>

#include <glog/logging.h>

#include "SipXCall.hh"

using namespace std;

namespace sipxsua {

using TClock = chrono::high_resolution_clock;
using TDuration = chrono::duration<float, milli>;

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

void Poller::runUntil(int timeout, int interval, PollPred pred) {
  int rc;
  auto tp = TClock::now();
  while (1) {
    if (refillFds()) {
      CHECK_ERR(rc = poll(fds, nfds, timeout));
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
            lock_guard<mutex> lk(SipXCall::instancesMutex);
            auto reader = SipXCall::findReader(pfd.fd);
            if (reader) {
              reader->read();
            }
          }
        } /// endfor
      } else {
        // Timeout!
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
