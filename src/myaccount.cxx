#include <iostream>

#include "myaccount.hxx"
#include "mycall.hxx"

void MyAccount::onRegState(pj::OnRegStateParam& prm) {
  pj::AccountInfo ai = getInfo();
  std::cout << (ai.regIsActive ? "*** Register:" : "*** Unregister:")
            << " code=" << prm.code << std::endl;
}

void MyAccount::onIncomingCall(pj::OnIncomingCallParam& iprm) {

  pj::Call* call = new MyCall(*this, iprm.callId);
  pj::CallOpParam prm;
  prm.statusCode = PJSIP_SC_OK;
  call->answer(prm);
}