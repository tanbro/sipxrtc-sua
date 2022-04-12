#include <iostream>

#include "globalvars.hxx"
#include "mycall.hxx"

static pj_pool_factory pool_factory;

MyCall::MyCall(pj::Account &acc, int call_id) : Call(acc, call_id) {
  pool = pj_pool_create(&pool_factory, "MyCallPool", 4096, 1024, NULL);
  // pjmedia_wav_writer_port_create
}

MyCall::~MyCall() { pj_pool_release(pool); }

void MyCall::onCallState(pj::OnCallStateParam &prm) {
  auto ci = getInfo();
  std::cout << ci.remoteUri << "onCallState: " << ci.stateText << std::endl;

  if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
    /// TODO: Schedule/Dispatch call deletion to another thread here
  }
}

void MyCall::onCallMediaState(pj::OnCallMediaStateParam &prm) {

  auto ci = getInfo();
  // Iterate all the call medias
  for (unsigned i = 0; i < ci.media.size(); i++) {
    if (ci.media[i].type == PJMEDIA_TYPE_AUDIO) {
      auto aud_med = getAudioMedia(i);

      // Connect the call audio media to sound device
      pj::AudDevManager &mgr = ep.audDevManager();
      aud_med.startTransmit(mgr.getPlaybackDevMedia());
      mgr.getCaptureDevMedia().startTransmit(aud_med);
    }
  }
}