#ifndef __sipxsua_SipXAccount__
#define __sipxsua_SipXAccount__ 1

#include <map>
#include <memory>
#include <utility>

#include <pjsua2.hpp>

namespace sipxsua {

using TCall = std::shared_ptr<pj::Call>;
using TCallMap = std::map<int, TCall>;

// Subclass to extend the Account and get notifications etc.
class SipXAccount : public pj::Account {
private:
  TCallMap callMap;

public:
  ~SipXAccount();
  virtual void onRegState(pj::OnRegStateParam &);
  virtual void onIncomingCall(pj::OnIncomingCallParam &);

  void removeCall(int);
};

} // namespace sipxsua

#endif
