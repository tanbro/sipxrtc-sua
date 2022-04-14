#include <iostream>

#include "SipXAccount.hxx"
#include "SipXCall.hxx"

namespace sipxsua {

void SipXAccount::onRegState(pj::OnRegStateParam &prm) {
  pj::AccountInfo ai = getInfo();
  std::cout << (ai.regIsActive ? "*** Register:" : "*** Unregister:")
            << " code=" << prm.code << std::endl;
}

void SipXAccount::onIncomingCall(pj::OnIncomingCallParam &iprm) {

  pj::Call *call = new SipXCall(*this, iprm.callId);
  pj::CallOpParam prm;
  prm.statusCode = PJSIP_SC_OK;
  call->answer(prm);
}

} // namespace sipxsua