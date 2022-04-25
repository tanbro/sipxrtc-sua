#include "SipXAccount.hh"

#include <glog/logging.h>

#include "SipXCall.hh"

using namespace std;

namespace sipxsua {

SipXAccount::~SipXAccount() { shutdown(); }

void SipXAccount::onRegState(pj::OnRegStateParam &prm) {
  auto ai = getInfo();
  LOG(INFO) << "[" << ai.id << "] onRegState" << endl
            << "  regIsActive=" << ai.regIsActive << endl
            << "  code:" << prm.code << endl
            << "  regStatus:" << ai.regStatus << " " << ai.regStatusText;
}

void SipXAccount::onIncomingCall(pj::OnIncomingCallParam &iprm) {

  auto call = make_shared<SipXCall>(*this, iprm.callId);
  auto insRes = callMap.insert(make_pair<int, TCall>(getId(), call));
  CHECK(insRes.second) << "重复的 Call ID " << getId();

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

void SipXAccount::removeCall(int callId) {
  CHECK_LT(0, callMap.erase(callId));
}

} // namespace sipxsua