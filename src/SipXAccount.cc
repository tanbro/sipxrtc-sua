#include "SipXAccount.hh"

#include <iostream>

#include <glog/logging.h>

#include "SipXCall.hh"

namespace sipxsua {

SipXAccount::~SipXAccount() { shutdown(); }

void SipXAccount::onRegState(pj::OnRegStateParam &prm) {
  auto ai = getInfo();
  LOG(INFO) << "[" << ai.id << "] onRegState\n"
            << "  regIsActive=" << ai.regIsActive << "\n"
            << "  code:" << prm.code << "\n"
            << "  regStatus:" << ai.regStatus << " " << ai.regStatusText;
}

void SipXAccount::onIncomingCall(pj::OnIncomingCallParam &iprm) {

  pj::Call *call = new SipXCall(*this, iprm.callId);

  auto ai = getInfo();
  auto ci = call->getInfo();

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