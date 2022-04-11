#ifndef __my_account_h__
#define __my_account_h__

#include <pjsua2.hpp>

// Subclass to extend the Account and get notifications etc.
class MyAccount : public pj::Account {
public:
    virtual void onRegState(pj::OnRegStateParam&);
    virtual void onIncomingCall(pj::OnIncomingCallParam&);
};

#endif
