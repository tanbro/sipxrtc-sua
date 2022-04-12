#ifndef __my_call_h__
#define __my_call_h__

#include <pjmedia.h>
#include <pjsua2.hpp>

class MyCall : public pj::Call {
public:
  MyCall(pj::Account &acc, int call_id = PJSUA_INVALID_ID);

  ~MyCall();
  virtual void onCallState(pj::OnCallStateParam &);
  virtual void onCallMediaState(pj::OnCallMediaStateParam &);

  pj_pool_t *pool;
  pjmedia_port *record_port;
};

#endif
