#include "SipXAccount.hh"

#include <glog/logging.h>

#include "SipXCall.hh"

namespace sipxsua {

using namespace std;

SipXAccount::~SipXAccount() { shutdown(); }

/**
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

  pj::CallOpParam prm(true);
  prm.statusCode = PJSIP_SC_UNWANTED;

  LOG(WARNING) << "[" << getId() << "] onIncomingCall"
               << " "
               << "(" << ci.id << "/" << ci.callIdString << "):"
               << " " << ci.remoteUri << " --> " << ci.localUri << " "
               << "hangup: " << prm.statusCode;

  call->hangup(prm);
}
*/

} // namespace sipxsua