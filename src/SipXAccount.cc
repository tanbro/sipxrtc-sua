#include "SipXAccount.hh"

#include <glog/logging.h>

#include "SipXCall.hh"

using namespace std;

namespace sipxsua {

SipXAccount::~SipXAccount() {
  auto ai = getInfo();
  VLOG(1) << "[" << ai.id << "] destruct ... shutdown ...";
  shutdown();
}

void SipXAccount::onRegState(pj::OnRegStateParam &prm) {
  auto ai = getInfo();
  LOG(INFO) << "[" << ai.id << "] onRegState" << endl
            << "  regIsActive=" << ai.regIsActive << endl
            << "  code:" << prm.code << endl
            << "  regStatus:" << ai.regStatus << " " << ai.regStatusText;
}

void SipXAccount::onIncomingCall(pj::OnIncomingCallParam &iprm) {
  auto call = SipXCall::createCall(*this, iprm.callId);

  auto ci = call->getInfo();
  auto ai = getInfo();

  pj::CallOpParam prm;
  prm.statusCode = PJSIP_SC_OK;

  LOG(INFO) << "[" << ai.id << "] onIncomingCall"
            << " "
            << "(" << ci.id << "/" << ci.callIdString << "):"
            << " " << ci.remoteUri << " --> " << ci.localUri << " "
            << "answer " << prm.statusCode;

  call->answer(prm);
}

} // namespace sipxsua