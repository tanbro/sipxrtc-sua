#ifndef __sipxsua_global_h__
#define __sipxsua_global_h__ 1

#include <pjsua2.hpp>

#include "SuaLogWriter.hh"
#include "SipXAccount.hh"
#include "Poller.hh"

namespace sipxsua {

extern pj::Endpoint ep;

extern SipXAccount account;

extern Poller poller;

extern SuaLogWriter suaLogWriter;

} // namespace sipxsua

#endif
