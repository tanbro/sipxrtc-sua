#ifndef __sipxsua_UdsBase_h
#define __sipxsua_UdsBase_h 1

namespace sipxsua {

/**
 * @brief UDS 的读写的基础类型
 *
 */
class UdsBase {
public:
  UdsBase(){};
  virtual ~UdsBase();

  virtual int activate();
  virtual void deactivate();

  virtual int getFd();

protected:
  int fd = -1;
};

}; // namespace sipxsua

#endif
