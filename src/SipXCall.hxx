#ifndef __SipXCall__
#define __SipXCall__

#include <memory>
#include <set>

#include <pjmedia.h>
#include <pjsua2.hpp>

#include "AudioMediaUdsWriter.hxx"

namespace sipxsua {

class SipXCall : public pj::Call {
public:
  SipXCall(pj::Account &acc, int call_id = PJSUA_INVALID_ID);

  ~SipXCall();
  virtual void onCallState(pj::OnCallStateParam &);
  virtual void onCallMediaState(pj::OnCallMediaStateParam &);

private:
  pj_caching_pool cachingPool;
  pj_pool_t *pool;
  pjmedia_port *record_port;

  AudioMediaUdsWriter *recorder = nullptr;
};

} // namespace sipxsua

#endif
