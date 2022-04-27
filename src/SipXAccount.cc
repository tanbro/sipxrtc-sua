#include "SipXAccount.hh"

#include <glog/logging.h>

#include "SipXCall.hh"

using namespace std;

namespace sipxsua {

SipXAccount::~SipXAccount() {
  VLOG(1) << "~[" << getId() << "] dtor";
  shutdown();
}

void SipXAccount::onRegState(pj::OnRegStateParam &prm) {
  auto ai = getInfo();
  LOG(INFO) << "[" << getId() << "] onRegState" << endl
            << "  regIsActive=" << ai.regIsActive << endl
            << "  code:" << prm.code << endl
            << "  regStatus:" << ai.regStatus << " " << ai.regStatusText;
}

void SipXAccount::onIncomingCall(pj::OnIncomingCallParam &iprm) {
  auto call = SipXCall::createCall(*this, iprm.callId);

  auto ci = call->getInfo();

  pj::CallOpParam prm;
  prm.statusCode = PJSIP_SC_UNWANTED;

  LOG(WARNING) << "[" << getId() << "] onIncomingCall"
               << " "
               << "(" << ci.id << "/" << ci.callIdString << "):"
               << " " << ci.remoteUri << " --> " << ci.localUri << " "
               << "answer: " << prm.statusCode;

  call->answer(prm);
}

} // namespace sipxsua