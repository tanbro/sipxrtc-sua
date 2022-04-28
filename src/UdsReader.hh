#ifndef __sipxsua_UdsReader_h__
#define __sipxsua_UdsReader_h__ 1

#include <sys/un.h>

#include <cstddef>
#include <string>

#include "UdsBase.hh"

namespace sipxsua {
class UdsReader : public UdsBase {
public:
  UdsReader(const std::string &path);
  virtual ~UdsReader() override;

  virtual ssize_t read(void *data, size_t length);

  virtual int open() override;
  virtual void close() override;

protected:
  sockaddr_un addr;
  std::string path;
};
}; // namespace sipxsua

#endif
