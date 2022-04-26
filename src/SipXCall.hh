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
  SipXCall(pj::Account &acc, int callId = PJSUA_INVALID_ID);
  ~SipXCall();

  static std::shared_ptr<SipXCall> createCall(pj::Account &acc,
                                              int callId = PJSUA_INVALID_ID);

  virtual void onCallState(pj::OnCallStateParam &);
  virtual void onCallMediaState(pj::OnCallMediaStateParam &);

  AudioMediaUdsReader *getReader();
  AudioMediaUdsWriter *getWriter();

  static std::set<int> getAllFds();
  static std::mutex instancesMutex;
  static std::set<std::shared_ptr<SipXCall>> instances;

  /**
   * @brief
   *
   * 注意 calls set 的线程安全问题。
   *
   * 应在 callMutex 保护下调用这个方法。
   *
   * @param fd
   * @return AudioMediaUdsReader*
   */
  static AudioMediaUdsReader *findReader(int fd);

  /**
   * @brief
   *
   * 注意 calls set 的线程安全问题。
   *
   * 应在 callMutex 保护下调用这个方法。
   *
   * @param fd
   * @return AudioMediaUdsWriter*
   */
  static AudioMediaUdsWriter *findWriter(int fd);

private:
  static bool internalReleaseCall(SipXCall *p);

  pj_caching_pool cachingPool;
  pj_pool_t *pool;
  pjmedia_port *record_port;

  AudioMediaUdsReader *reader = nullptr;
  AudioMediaUdsWriter *writer = nullptr;
};

} // namespace sipxsua

#endif
