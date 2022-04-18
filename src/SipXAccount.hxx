#ifndef __sipxsua_SipXAccount__
#define __sipxsua_SipXAccount__

#include <pjsua2.hpp>

namespace sipxsua {

// Subclass to extend the Account and get notifications etc.
class SipXAccount : public pj::Account {
public:
  virtual void onRegState(pj::OnRegStateParam &);
  virtual void onIncomingCall(pj::OnIncomingCallParam &);
};

} // namespace sipxsua

#endif
