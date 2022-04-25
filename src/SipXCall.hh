#ifndef __sipxsua_SipXCall__
#define __sipxsua_SipXCall__ 1

#include <memory>
#include <mutex>
#include <set>

#include <pjmedia.h>
#include <pjsua2.hpp>

#include "AudioMediaUdsReader.hh"
#include "AudioMediaUdsWriter.hh"

namespace sipxsua {

using PCall = std::shared_ptr<pj::Call>;

class SipXCall : public pj::Call {
public:
  SipXCall(pj::Account &acc, int callId = PJSUA_INVALID_ID);
  ~SipXCall();

  static PCall createCall(pj::Account &acc, int callId = PJSUA_INVALID_ID);

  virtual void onCallState(pj::OnCallStateParam &);
  virtual void onCallMediaState(pj::OnCallMediaStateParam &);

  AudioMediaUdsReader *getReader();
  AudioMediaUdsWriter *getWriter();

private:
  static bool internalReleaseCall(SipXCall *p);

  pj_caching_pool cachingPool;
  pj_pool_t *pool;
  pjmedia_port *record_port;

  AudioMediaUdsReader *reader = nullptr;
  AudioMediaUdsWriter *writer = nullptr;

  static std::mutex _callsMtx;
  static std::set<PCall> _calls;
};

} // namespace sipxsua

#endif
