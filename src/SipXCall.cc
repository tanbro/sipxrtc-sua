#include "SipXCall.hh"

#include <glog/logging.h>
#include <pjmedia/mem_port.h>

#include "SipXAccount.hh"
#include "global.hh"

using namespace std;
using namespace pj;

namespace sipxsua {

static pj_pool_factory pool_factory;

SipXCall::SipXCall(Account &acc, int call_id) : Call(acc, call_id) {
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "SipXCall", 512, 512, NULL);
  CHECK_NOTNULL(pool);
  int accId = acc.getId();
}

SipXCall::~SipXCall() {
  auto ci = getInfo();
  LOG(WARNING) << "[" << ci.accId << "]"
               << " "
               << "(" << ci.id << "/" << ci.callIdString << "): "
               << "~ Destruct ... " << ci.state << " " << ci.stateText;
  if (reader != nullptr) {
    delete reader;
  }
  if (writer != nullptr) {
    delete writer;
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

mutex SipXCall::_callsMtx;
set<TCallPtr> SipXCall::_calls;

TCallPtr SipXCall::createCall(pj::Account &acc, int callId) {
  auto call = make_shared<SipXCall>(acc, callId);
  {
    lock_guard<mutex> lk(_callsMtx);
    auto result = _calls.insert(call);
    CHECK(result.second) << ": insertion for calls set not took place";
  }
  return call;
}

bool SipXCall::internalReleaseCall(SipXCall *p) {
  lock_guard<mutex> lk(_callsMtx);
  set<TCallPtr>::const_iterator found = _calls.end();
  for (auto it = _calls.begin(); it != _calls.end(); ++it) {
    if (p == (SipXCall *)it->get()) {
      found = it;
      break;
    }
  }
  if (found == _calls.end()) {
    return false;
  }
  _calls.erase(found);
  return true;
}

void SipXCall::onCallState(OnCallStateParam &prm) {
  auto ci = getInfo();

  LOG(INFO) << "[" << ci.accId << "]"
            << " "
            << "(" << ci.id << "/" << ci.callIdString << "): CallState ... "
            << ci.state << " " << ci.stateText;

  if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
    /// TODO: Schedule/Dispatch call deletion to another thread here
    internalReleaseCall(this);
  }
}

void SipXCall::onCallMediaState(OnCallMediaStateParam &prm) {
  AudDevManager &mgr = ep.audDevManager();
  auto ci = getInfo();

  auto peerMedia = getAudioMedia(-1); // 收到的来自远端的声音媒体
  auto peerAuFmt = peerMedia.getPortInfo().format;

  LOG(INFO) << "[" << ci.accId << "]"
            << " "
            << "(" << ci.id << "/" << ci.callIdString << "):"
            << " "
            << "CallMedia start!"
            << "\n"
            << "  bits_per_sample=" << peerAuFmt.bitsPerSample << "\n"
            << "  channel=" << peerAuFmt.channelCount << "\n"
            << "  sample_rate=" << peerAuFmt.clockRate << "\n"
            << "  avg_bps=" << peerAuFmt.avgBps << "\n"
            << "  max_bps=" << peerAuFmt.maxBps;

  DVLOG(2) << "create UDS reader";
  if (reader != nullptr) {
    delete reader;
    reader = nullptr;
  }
  reader = new AudioMediaUdsReader();
  struct MediaFormatAudio rtcAuFmt;
  rtcAuFmt.channelCount = 1;
  rtcAuFmt.clockRate = 48000;
  rtcAuFmt.bitsPerSample = 16;
  rtcAuFmt.frameTimeUsec = 20000; // 20 毫秒
  reader->createPlayer(rtcAuFmt, "/tmp/sipxrtp-trtc.sock", 48000, 20);

  DVLOG(2) << "create UDS writer";
  if (writer != nullptr) {
    delete writer;
    writer = nullptr;
  }
  writer = new AudioMediaUdsWriter();
  writer->createRecorder(peerAuFmt, "/tmp/sipxrtp-sua.sock", 48000, 20);

  DVLOG(2) << "UDS Reader startTransmit";
  reader->startTransmit(peerMedia);
  DVLOG(2) << "UDS Writer startTransmit";
  peerMedia.startTransmit(*writer);
}

AudioMediaUdsReader *SipXCall::getReader() { return reader; }

AudioMediaUdsWriter *SipXCall::getWriter() { return writer; }

} // namespace sipxsua
