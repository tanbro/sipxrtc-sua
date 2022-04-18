#include <iostream>
#include <sstream>

#include <pjmedia/mem_port.h>

#include "SipXCall.hxx"
#include "global.hxx"

namespace sipxsua {

static pj_pool_factory pool_factory;

SipXCall::SipXCall(pj::Account &acc, int call_id) : Call(acc, call_id) {
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "SipXCall", 512, 512, NULL);
}

SipXCall::~SipXCall() {
  if (recorder != nullptr) {
    delete recorder;
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

void SipXCall::onCallState(pj::OnCallStateParam &prm) {
  auto ci = getInfo();
  std::cout << ci.remoteUri << "onCallState: " << ci.stateText << std::endl;

  if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
    /// TODO: Schedule/Dispatch call deletion to another thread here
  }
}

void SipXCall::onCallMediaState(pj::OnCallMediaStateParam &prm) {
  pj::AudDevManager &mgr = ep.audDevManager();
  auto ci = getInfo();

  auto remote_med = getAudioMedia(-1); // 收到的来自远端的声音媒体
  // auto local_med =
  //     ep.audDevManager().getCaptureDevMedia(); // 来自本地采集的声音媒体
  // 本地 x 远端
  // remote_med.startTransmit(mgr.getPlaybackDevMedia());
  // local_med.startTransmit(remote_med);

  // auto remotePortInfo = remote_med.getPortInfo();
  // auto remotePortFmt = remotePortInfo.format;

  if (recorder != nullptr) {
    delete recorder;
    recorder = nullptr;
  }
  recorder = new AudioMediaUdsWriter();
  auto audioFormat = getAudioMedia(-1).getPortInfo().format;
  recorder->createRecorder(audioFormat, "/tmp/sipxrtp-sip.sock", 48000, 200);
  remote_med.startTransmit(*recorder);
}

} // namespace sipxsua
