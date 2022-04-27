#ifndef __sipxsua_UdsWriter_h__
#define __sipxsua_UdsWriter_h__ 1

#include <sys/un.h>

#include <cstddef>
#include <string>

#include "UdsBase.hh"

namespace sipxsua {
class UdsWriter : public UdsBase {
public:
  UdsWriter(const std::string &path);

  /**
   * @brief
   *
   * 注意：我们没有任务队列，就把写入直接放在现场执行好了！
   *
   * @param data
   * @param length
   * @return ssize_t
   */
  virtual ssize_t write(void *data, size_t length);

protected:
  sockaddr_un addr;
  std::string path;
};
}; // namespace sipxsua

#endif
