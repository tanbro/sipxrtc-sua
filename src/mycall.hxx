#ifndef __my_call_h__
#define __my_call_h__

#include <pjsua2.hpp>

class MyCall : public pj::Call {
 public:
  MyCall(pj::Account& acc, int call_id = PJSUA_INVALID_ID)
      : Call(acc, call_id){};

  ~MyCall(){};
  virtual void onCallState(pj::OnCallStateParam&);
  virtual void onCallMediaState(pj::OnCallMediaStateParam&);
};

#endif
