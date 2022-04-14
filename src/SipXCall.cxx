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
  auto local_med =
      ep.audDevManager().getCaptureDevMedia(); // 来自本地采集的声音媒体

  // Connect the call audio media to sound device
  // remote_med.startTransmit(mgr.getPlaybackDevMedia());
  // local_med.startTransmit(remote_med);

  ////////////////////////////////////////////////////////////////////////////////
  // // 很上层的 C++ 接口，只能到文件
  // // 录制: 远端 Audio --> 文件
  // std::ostringstream file_name;
  // file_name << "tmp/rec-" << ci.accId << "-" << ci.callIdString <<
  // ci.remoteUri
  //           << ".wav";
  // if (recorder != nullptr) {
  //   delete recorder;
  // }
  // recorder = new pj::AudioMediaRecorder();
  // recorder->createRecorder(file_name.str());
  // remote_med.startTransmit(*recorder);
  // // 播放: 文件 --> 本地采集
  // if (player != nullptr) {
  //   delete player;
  // }
  // player = new pj::AudioMediaPlayer();
  // player->createPlayer("/home/liuxy/音乐/song-16bit-8k.wav");
  // player->startTransmit(remote_med);
  ////////////////////////////////////////////////////////////////////////////////

  auto remotePortInfo = remote_med.getPortInfo();
  auto remotePortFmt = remotePortInfo.format;
  pjsua_conf_port_info remote_port_info;
  pjsua_conf_get_port_info(remotePortInfo.portId, &remote_port_info);

  if (recorder != nullptr) {
    delete recorder;
    recorder = nullptr;
  }
  recorder = new AudioMediaUdsWriter();
  recorder->createRecorder(this, "/tmp/sipxrtp-sip.sock", 44100, 1, 160, 16000);
  remote_med.startTransmit(*recorder);

  ////////////////////////////////////////////////////////////////////////////////
  // // 尝试 mem_port
  // pjmedia_port* port;

  // auto remotePortInfo = remote_med.getPortInfo();
  // auto remotePortFmt = remotePortInfo.format;

  // pjsua_conf_port_info remote_port_info;
  // pjsua_conf_get_port_info(remotePortInfo.portId, &remote_port_info);

  // pjmedia_mem_capture_create(pool, rec_buf, rec_buf_sz,
  // remotePortFmt.clockRate,
  //                            remotePortFmt.channelCount,
  //                            remote_port_info.samples_per_frame,
  //                            remotePortFmt.bitsPerSample, 0, &port);
  // pjsua_conf_port_id rec_port_id;
  // pjsua_conf_add_port(pool, port, &rec_port_id);
  // assert(rec_port_id != PJSUA_INVALID_ID);
  // pjsua_conf_connect(remote_med.getPortInfo().portId, rec_port_id);
}

} // namespace sipxsua