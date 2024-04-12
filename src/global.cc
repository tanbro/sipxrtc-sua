#include "global.hh"

namespace sipxsua {

bool interrupted = false;

pj::Endpoint *ep;

// 我们有且仅有一个外呼的 call, 放在这个全局变量!
std::shared_ptr<SipXCall> theCall = nullptr;

EventPub *eventPub = nullptr;

} // namespace sipxsua