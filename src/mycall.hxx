#ifndef __my_call_h__
#define __my_call_h__

#include <memory>
#include <set>

#include <pjmedia.h>
#include <pjsua2.hpp>

#include "AudioMediaLocalDataGramRecorder.hxx"

class MyCall : public pj::Call {
 public:
  MyCall(pj::Account& acc, int call_id = PJSUA_INVALID_ID);

  ~MyCall();
  virtual void onCallState(pj::OnCallStateParam&);
  virtual void onCallMediaState(pj::OnCallMediaStateParam&);

 private:
  pj_caching_pool cachingPool;
  pj_pool_t* pool;
  pjmedia_port* record_port;

  pj::AudioMediaPlayer* player = nullptr;

  AudioMediaLocalDataGramRecorder* recorder = nullptr;

  size_t rec_buf_sz = 1024 * 1024;
  void* rec_buf;
};

#endif
