#include "EventPub.hh"

#include <fcntl.h>

#include <cerrno>
#include <climits>
#include <cstdio>
#include <cstring>

#include <glog/logging.h>

using namespace std;

namespace sipxsua {

static const string trimStr(const std::string &s);

EventPub::EventPub(const string &path) { _path = path; }

EventPub::~EventPub() {
  if (!(_fd < 0)) {
    close();
  }
}

int EventPub::open() {
  CHECK_GT(0, _fd);
  CHECK_ERR(_fd = ::open(_path.c_str(), O_WRONLY | O_NONBLOCK));
  LOG(INFO) << "open " << _fd << ":" << _path;
  return _fd;
}

void EventPub::close() {
  LOG(INFO) << "close " << _fd << ":" << _path;
  CHECK_LE(0, _fd);
  CHECK_ERR(::close(_fd));
  _fd = -1;
}

int EventPub::getFd() { return _fd; }

void EventPub::pub(const string &msg) {
  CHECK_LE(0, _fd);
  ostringstream oss;
  oss << trimStr(msg) << endl;
  CHECK_GT(PIPE_BUF, oss.str().length());
  char *buf = (char *)calloc(oss.str().length() + 1, sizeof(char));
  strncpy(buf, oss.str().c_str(), oss.str().length());
  VLOG(1) << "pub: " << buf;
  {
    lock_guard<mutex> lk(_mtx);
    CHECK_ERR(::write(_fd, buf, oss.str().length() + 1));
  }
  free(buf);
}

const string trimStr(const std::string &s) {
  string s1 = s;
  while (!s1.empty() && isspace(s1.back()))
    s1.pop_back();
  while (!s1.empty() && isspace(s1.front()))
    s1.erase(0, 1);
  return s1;
}

} // namespace sipxsua