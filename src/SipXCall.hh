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

class SipXCall : public pj::Call {

public:
  SipXCall(pj::Account &acc);
  ~SipXCall();

  static std::unique_ptr<SipXCall> instance;

  static void create(pj::Account &acc);

  virtual void onCallState(pj::OnCallStateParam &) override;
  virtual void onCallMediaState(pj::OnCallMediaStateParam &) override;

  AudioMediaUdsReader *getReader() { return reader; };
  AudioMediaUdsWriter *getWriter() { return writer; };

private:
  void destroyPlayerAndRecorder();

  pj_caching_pool cachingPool;
  pj_pool_t *pool;
  pjmedia_port *record_port;

  AudioMediaUdsReader *reader = nullptr;
  AudioMediaUdsWriter *writer = nullptr;
};

} // namespace sipxsua

#endif
