#ifndef __sipxsua_SipXAccount__
#define __sipxsua_SipXAccount__ 1

#include <pjsua2.hpp>

namespace sipxsua {

// Subclass to extend the Account and get notifications etc.
class SipXAccount : public pj::Account {
public:
  ~SipXAccount();
  virtual void onRegState(pj::OnRegStateParam &) override;
  virtual void onIncomingCall(pj::OnIncomingCallParam &) override;
};

} // namespace sipxsua

#endif
