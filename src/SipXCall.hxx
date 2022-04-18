#ifndef __sipxsua_SipXCall__
#define __sipxsua_SipXCall__

#include <memory>
#include <set>

#include <pjmedia.h>
#include <pjsua2.hpp>

#include "AudioMediaUdsReader.hxx"
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

  AudioMediaUdsReader *reader = nullptr;
  AudioMediaUdsWriter *writer = nullptr;
};

} // namespace sipxsua

#endif
