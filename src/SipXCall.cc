#include "SipXCall.hh"

#include <csignal>

#include <glog/logging.h>
#include <pjmedia/mem_port.h>

#include "IpcFlags.hh"
#include "SipXAccount.hh"
#include "global.hh"

using namespace std;
using namespace pj;

namespace sipxsua {

SipXCall::SipXCall(Account &acc, int call_id) : Call(acc, call_id) {
  LOG(INFO) << "ctor " << getId();
  _isIncoming = call_id != PJSUA_INVALID_ID;
}

SipXCall::~SipXCall() {
  LOG(INFO) << "~dtor " << getId();
  destroyPlayerAndRecorder();
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

  if (eventPub) {
    ostringstream oss;
    oss << "onCallState: " << ci.state << " " << ci.stateText;
    eventPub->pub(oss.str());
  }

  if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
    /// ATTENTION: Schedule/Dispatch call deletion to another thread here
    /// `theCall` 是全局的 shared_ptr，不会自动释放！
    internalReleaseCall(this);
    if (this == theCall.get()) {
      interrupted = true;
    }
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

  if (eventPub) {
    ostringstream oss;
    oss << "onCallMediaState: " << ci.state << " " << ci.stateText;
    eventPub->pub(oss.str());
  }

  destroyPlayerAndRecorder();

  reader = new AudioMediaUdsReader(FLAGS_aud_capture_path);
  struct MediaFormatAudio recvAudFmt;
  recvAudFmt.channelCount = 1;
  recvAudFmt.clockRate = FLAGS_aud_capture_samplerate;
  recvAudFmt.bitsPerSample = 16;
  recvAudFmt.frameTimeUsec = FLAGS_aud_capture_frametime * 1000; // 毫秒转微秒
  reader->createPlayer(recvAudFmt);

  writer = new AudioMediaUdsWriter(FLAGS_aud_playback_path);
  writer->createRecorder(peerAuFmt, FLAGS_aud_playback_samplerate,
                         FLAGS_aud_playback_frametime,
                         FLAGS_aud_playback_resample_level);

  /// [Playerback] reader ==> peer
  /// [Capture] peer ==> writer
  DVLOG(2) << "UDS Reader startTransmit";
  reader->startTransmit(peerMedia);
  DVLOG(2) << "UDS Writer startTransmit";
  peerMedia.startTransmit(*writer);
}

void SipXCall::destroyPlayerAndRecorder() {
  if (reader) {
    delete reader;
    reader = nullptr;
  }
  if (writer) {
    delete writer;
    writer = nullptr;
  }
}

bool SipXCall::isIncoming() { return _isIncoming; }

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
