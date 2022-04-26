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
  LOG(WARNING) << "~[" << ci.accId << "]"
               << "dtor "
               << "(" << getId() << "/" << ci.callIdString << ")";
  if (reader != nullptr) {
    delete reader;
  }
  if (writer != nullptr) {
    delete writer;
  }
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

mutex SipXCall::instancesMutex;
set<shared_ptr<SipXCall>> SipXCall::instances;

shared_ptr<SipXCall> SipXCall::createCall(pj::Account &acc, int callId) {
  auto call = make_shared<SipXCall>(acc, callId);
  {
    lock_guard<mutex> lk(instancesMutex);
    auto result = instances.insert(call);
    CHECK(result.second) << ": insertion for calls set not took place";

    VLOG(1) << "call [" << call->getId()
            << "] created. instances.size():" << instances.size();
  }
  return call;
}

bool SipXCall::internalReleaseCall(SipXCall *p) {
  lock_guard<mutex> lk(instancesMutex);
  auto cid = p->getId();
  auto found = instances.end();
  for (auto it = instances.begin(); it != instances.end(); ++it) {
    if (p == (SipXCall *)it->get()) {
      found = it;
      break;
    }
  }
  if (found == instances.end()) {
    LOG(ERROR) << "Call [" << cid << "] not in instances set";
  }
  instances.erase(found);
  VLOG(1) << "call [" << cid
          << "] released. instances.size():" << instances.size();

  return true;
}

void SipXCall::onCallState(OnCallStateParam &prm) {
  auto ci = getInfo();

  LOG(INFO) << "[" << ci.accId << "]"
            << " "
            << "(" << getId() << "/" << ci.callIdString << "): CallState ... "
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
            << "(" << getId() << "/" << ci.callIdString << "):"
            << " "
            << "MediaState:" << endl
            << "  bits_per_sample=" << peerAuFmt.bitsPerSample << "\n"
            << "  channel=" << peerAuFmt.channelCount << "\n"
            << "  sample_rate=" << peerAuFmt.clockRate << "\n"
            << "  avg_bps=" << peerAuFmt.avgBps << "\n"
            << "  max_bps=" << peerAuFmt.maxBps;

  if (reader != nullptr) {
    delete reader;
    reader = nullptr;
  }
  reader = new AudioMediaUdsReader("/tmp/sipxrtp-trtc.sock");
  struct MediaFormatAudio rtcAuFmt;
  rtcAuFmt.channelCount = 1;
  rtcAuFmt.clockRate = 48000;
  rtcAuFmt.bitsPerSample = 16;
  rtcAuFmt.frameTimeUsec = 20000; // 20 毫秒
  reader->createPlayer(rtcAuFmt, 48000, 20);

  DVLOG(2) << "create UDS writer";
  if (writer != nullptr) {
    delete writer;
    writer = nullptr;
  }
  writer = new AudioMediaUdsWriter("/tmp/sipxrtp-sua.sock");
  writer->createRecorder(peerAuFmt, 48000, 20);

  /// [Playerback] reader ==> peer
  /// [Capture] peer ==> writer
  DVLOG(2) << "UDS Reader startTransmit";
  reader->startTransmit(peerMedia);
  DVLOG(2) << "UDS Writer startTransmit";
  peerMedia.startTransmit(*writer);
}

AudioMediaUdsReader *SipXCall::getReader() { return reader; }

AudioMediaUdsWriter *SipXCall::getWriter() { return writer; }

set<int> SipXCall::getAllFds() {
  set<int> result;
  {
    lock_guard<mutex> lk(instancesMutex);
    for (auto it = instances.begin(); it != instances.end(); ++it) {
      auto call = *it;
      if (call->reader) {
        result.insert(call->reader->getFd());
      }
      if (call->writer) {
        result.insert(call->writer->getFd());
      }
    }
  }
  return result;
}

AudioMediaUdsReader *SipXCall::findReader(int fd) {
  for (auto it = instances.begin(); it != instances.end(); ++it) {
    auto call = *it;
    if (call->reader) {
      if (fd == call->reader->getFd()) {
        return call->reader;
      }
    }
  }
  return NULL;
}

AudioMediaUdsWriter *SipXCall::findWriter(int fd) {
  for (auto it = instances.begin(); it != instances.end(); ++it) {
    auto call = *it;
    if (call->writer) {
      if (fd == call->writer->getFd()) {
        return call->writer;
      }
    }
  }
  return NULL;
}

} // namespace sipxsua
