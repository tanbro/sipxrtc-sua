#ifndef __sipxsua_global_h__
#define __sipxsua_global_h__ 1

#include <memory>

#include <pjsua2.hpp>

#include <SipXAccount.hh>
#include "SuaLogWriter.hh"

namespace sipxsua {

extern pj::Endpoint ep;

// extern std::shared_ptr<SipXAccount> sipAcc;
extern SipXAccount sipAcc;

extern SuaLogWriter suaLogWriter;

}

#endif