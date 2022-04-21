#include <algorithm>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>

#include <pjsua2.hpp>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "SipXAccount.hh"
#include "SipXCall.hh"
#include "global.hh"

using namespace std;
using namespace sipxsua;

static bool running = true;

void sig_int_handler(int dummy) { running = false; }

int main(int argc, char *argv[]) {
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  LOG(INFO) << "Staring";

  ep.libCreate();

  // Initialize endpoint
  pj::EpConfig ep_cfg;
  // ep_cfg.uaConfig.threadCnt = 4;
  // ep_cfg.medConfig.threadCnt = 4;
  // ep_cfg.logConfig.consoleLevel = 5;
  // ep_cfg.logConfig.level = 4;
  // ep_cfg.logConfig.filename = "sua.log";
  // ep_cfg.logConfig.writer = &suaLogWriter;
  ep.libInit(ep_cfg);

  auto codecs = ep.codecEnum2();
  sort(codecs.begin(), codecs.end(), [](pj::CodecInfo a, pj::CodecInfo b) {
    return a.priority > b.priority;
  });
  for (auto it = codecs.begin(); it < codecs.end(); ++it) {
    VLOG(1) << "codec [" << (unsigned)it->priority << "] " << it->codecId;
  }

  // Create SIP transport. Error handling sample is shown
  try {
    pj::TransportConfig cfg;
    cfg.port = 5060;
    ep.transportCreate(PJSIP_TRANSPORT_UDP, cfg);
  } catch (pj::Error &err) {
    cout << err.info() << endl;
    return 1;
  }

  // Start the library (worker threads etc)
  ep.libStart();

  string line;

  SipXAccount *acc = nullptr;
  {
    // 本地账户
    pj::AccountConfig acc_cfg;
    acc_cfg.idUri = "sip:0.0.0.0";
    // acc_cfg.idUri = "sip:8007@192.168.2.202";
    // acc_cfg.regConfig.registrarUri = "sip:192.168.2.202";
    // acc_cfg.sipConfig.authCreds.push_back(
    //     pj::AuthCredInfo("digest", "*", "8007", 0, "hesong"));
    // Create the account
    acc = new SipXAccount();
    acc->create(acc_cfg, true);
  }

  // 使用空设备
  pj::AudDevManager &aud_dev_mgr = ep.audDevManager();
  aud_dev_mgr.setNullDev();
  assert(0 == aud_dev_mgr.getDevCount());

  cout << "输入要呼叫的 SIP URI:" << endl;
  getline(cin, line);
  if (!line.empty()) {
    pj::Call *call = new SipXCall(*acc);
    pj::CallOpParam prm(true); // Use default call settings
    try {
      call->makeCall(line, prm);
    } catch (pj::Error &err) {
      cout << err.info() << endl;
    }
  }

  //////////////
  printf("ctrl-c 退出\n");
  signal(SIGINT, sig_int_handler);
  while (running) {
    sleep(1);
  }
  //////////////////

  // Delete the account. This will unregister from server
  if (acc != nullptr) {
    delete acc;
  }
  ep.hangupAllCalls();
  ep.libDestroy();

  //////////////////
  return 0;
}
