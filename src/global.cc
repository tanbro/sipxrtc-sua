#include "global.hh"

namespace sipxsua {

bool interrupted = false;

pj::Endpoint ep;

SipXAccount account;

std::shared_ptr<SipXCall> theCall;

Poller poller;

SuaLogWriter suaLogWriter;

} // namespace sipxsua