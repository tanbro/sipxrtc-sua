#include <iostream>
#include <sstream>

#include "globalvars.hxx"
#include "mycall.hxx"

static pj_pool_factory pool_factory;

MyCall::MyCall(pj::Account &acc, int call_id) : Call(acc, call_id) {
  pj_caching_pool_init(&cachingPool, NULL, 0);
  pool = pj_pool_create(&cachingPool.factory, "MyCall", 512, 512, NULL);
  // pjmedia_wav_writer_port_create(pool = pool, "rec.wav", 8000, 1, 8000, 16,
  //                                PJMEDIA_FILE_WRITE_PCM, -1, &record_port);

  recorder = new pj::AudioMediaRecorder();
  player = new pj::AudioMediaPlayer();
}

MyCall::~MyCall() {
  delete recorder;
  delete player;
  pj_pool_release(pool);
  pj_caching_pool_destroy(&cachingPool);
}

void MyCall::onCallState(pj::OnCallStateParam &prm) {
  auto ci = getInfo();
  std::cout << ci.remoteUri << "onCallState: " << ci.stateText << std::endl;

  if (ci.state == PJSIP_INV_STATE_DISCONNECTED) {
    /// TODO: Schedule/Dispatch call deletion to another thread here
  }
}

void MyCall::onCallMediaState(pj::OnCallMediaStateParam &prm) {
  pj::AudDevManager &mgr = ep.audDevManager();
  auto ci = getInfo();

  auto remote_med = getAudioMedia(-1); // 收到的来自远端的声音媒体
  auto local_med =
      ep.audDevManager().getCaptureDevMedia(); // 来自本地采集的声音媒体

  // Connect the call audio media to sound device
  remote_med.startTransmit(mgr.getPlaybackDevMedia());
  local_med.startTransmit(remote_med);

  // 录制: 远端 Audio --> 文件
  std::ostringstream file_name;
  file_name << "tmp/rec-" << ci.accId << "-" << ci.callIdString << ci.remoteUri
            << ".wav";
  recorder->createRecorder(file_name.str());
  remote_med.startTransmit(*recorder);

  // 播放: 文件 --> 本地采集
  player->createPlayer("/home/liuxy/音乐/song-16bit-8k.wav");
  player->startTransmit(remote_med);
}