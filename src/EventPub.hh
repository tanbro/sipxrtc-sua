#ifndef __sipxsua_EventPub_h__
#define __sipxsua_EventPub_h__ 1

#include <mutex>
#include <string>

namespace sipxsua {

class EventPub {
public:
  EventPub(const std::string &path);
  ~EventPub();

  int open();
  void close();

  int getFd();

  void pub(const std::string &msg);

private:
  std::string _path;
  int _fd = -1;
  std::mutex _mtx;
};

}; // namespace sipxsua

#endif