#ifndef __sipxsua_Poller_h__
#define __sipxsua_Poller_h__ 1

#include <sys/poll.h>

#include <cstddef>

#include <pjsua2.hpp>

#ifndef SIPXSUA_MAX_POLL_FDS
#define SIPXSUA_MAX_POLL_FDS PJSUA_MAX_CALLS
#endif

namespace sipxsua {

/**
 * @brief
 *
 * ??? 注意：
 *
 * 发送器是 UnixDomainSocket 的 UDP，POLL 会认为它始终可以
 * POLLOUT，我们就不在这里处理了，而是让它现场写入！
 *
 */

class Poller {
public:
  Poller(){};
  ~Poller(){};

  void runForever();
  int refillFds();
  size_t performOnce(int timeout);

private:
  pollfd fds[SIPXSUA_MAX_POLL_FDS];
  int nfds = 0;
};

}; // namespace sipxsua

#endif