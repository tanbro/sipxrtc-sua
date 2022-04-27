#ifndef __sipxsua_global_h__
#define __sipxsua_global_h__ 1

#include <memory>

#include <pjsua2.hpp>

#include "Poller.hh"
#include "SipXAccount.hh"
#include "SipXCall.hh"
#include "SuaLogWriter.hh"

namespace sipxsua {

extern bool interrupted;

// 有且仅有一个 SIP stack
extern pj::Endpoint ep;

// 有且仅有一个 local 账户
extern SipXAccount account;

// 有且仅有一个 local 账户下的呼出 Call
extern std::shared_ptr<SipXCall> theCall;

extern Poller poller;

extern SuaLogWriter suaLogWriter;

} // namespace sipxsua

#endif
