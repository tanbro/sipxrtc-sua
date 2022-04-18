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
  if (reader != nullptr) {
    delete reader;
  }
  if (writer != nullptr) {
    delete writer;
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

  auto peerMedia = getAudioMedia(-1); // 收到的来自远端的声音媒体
  auto peerAuFmt = peerMedia.getPortInfo().format;

  if (reader != nullptr) {
    delete reader;
    reader = nullptr;
  }
  reader = new AudioMediaUdsReader();
  struct pj::MediaFormatAudio rtcAuFmt;
  rtcAuFmt.channelCount = 1;
  rtcAuFmt.clockRate = 48000;
  rtcAuFmt.bitsPerSample = 16;
  rtcAuFmt.frameTimeUsec = 20000; // 20 毫秒
  reader->createPlayer(rtcAuFmt, "/tmp/sipxrtp-trtc.sock", 48000, 20);

  if (writer != nullptr) {
    delete writer;
    writer = nullptr;
  }
  writer = new AudioMediaUdsWriter();
  writer->createRecorder(peerAuFmt, "/tmp/sipxrtp-sua.sock", 48000, 20);

  reader->startTransmit(peerMedia);
  peerMedia.startTransmit(*writer);
}

} // namespace sipxsua
