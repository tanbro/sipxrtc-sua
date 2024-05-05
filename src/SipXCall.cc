#include "SipXCall.hh"

#include <csignal>

#include <glog/logging.h>
#include <pjmedia/mem_port.h>

#include "IpcFlags.hh"
#include "SipXAccount.hh"
#include "global.hh"

namespace sipxsua {

using namespace std;
using namespace pj;

SipXCall::SipXCall(Account &acc) : Call(acc, PJSUA_INVALID_ID) {
  LOG(INFO) << "ctor " << getId();
}

SipXCall::~SipXCall() {
  LOG(INFO) << "~dtor " << getId();
  destroyPlayerAndRecorder();
}

std::unique_ptr<SipXCall> SipXCall::instance = nullptr;

void SipXCall::createCall(pj::Account &acc) {
  if (instance == nullptr) {
    instance = make_unique<SipXCall>(acc);
  } else {
    throw runtime_error("SipXCall instance exsited already.");
  }
}

void SipXCall::onCallState(OnCallStateParam &prm) {
  auto ci = getInfo();

  LOG(INFO) << "[" << ci.accId << "]"
            << " "
            << "(" << getId() << "/" << ci.callIdString << "): CallState ... "

            << " " << ci.state

            << " " << ci.stateText

            << " " << ci.lastStatusCode

            << " " << ci.lastReason;

  if (eventPub) {
    ostringstream oss;
    oss << "onCallState:"

        << " " << ci.state

        << " " << ci.stateText

        << " " << ci.lastStatusCode

        << " " << ci.lastReason;

    eventPub->pub(oss.str());
  }

  if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
    /// ATTENTION: Schedule/Dispatch call deletion to another thread here
    /// 设置全局标记，让主线程退出，在退出时释放这个对象。
    LOG(WARNING) << "DISCONNECTED";
    interrupted = true;
  }
}

void SipXCall::onCallMediaState(OnCallMediaStateParam &prm) {
  AudDevManager &mgr = ep->audDevManager();
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
    oss << "onCallMediaState: ";
    eventPub->pub(oss.str());
  }

  // 猜测：在 Media 改变之后，需要重复创建方可成功桥接
  destroyPlayerAndRecorder();

  VLOG(1) << "create UdsReader";
  reader = new AudioMediaUdsReader(FLAGS_aud_capture_path);
  struct MediaFormatAudio recvAudFmt;
  recvAudFmt.channelCount = 1;
  recvAudFmt.clockRate = FLAGS_aud_capture_samplerate;
  recvAudFmt.bitsPerSample = 16;
  recvAudFmt.frameTimeUsec = FLAGS_aud_capture_frametime * 1000; // 毫秒转微秒
  reader->createPlayer(recvAudFmt);

  VLOG(1) << "create UdsWriter";
  writer = new AudioMediaUdsWriter(FLAGS_aud_playback_path);
  writer->createRecorder(peerAuFmt, FLAGS_aud_playback_samplerate,
                         FLAGS_aud_playback_frametime,
                         FLAGS_aud_playback_resample_level);

  /// [Playerback] reader ==> peer
  /// [Capture] peer ==> writer

  DVLOG(2) << "UdsReader startTransmit";
  reader->startTransmit(peerMedia);

  DVLOG(2) << "UdsWriter startTransmit";
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

} // namespace sipxsua
