#include <algorithm>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <pjsua2.hpp>

#include "SipXAccount.hxx"
#include "SipXCall.hxx"
#include "global.hxx"

using namespace sipxsua;

static bool running = true;

void sig_int_handler(int dummy) { running = false; }

int main(int argc, char *argv[]) {
  ep.libCreate();

  // Initialize endpoint
  pj::EpConfig ep_cfg;
  ep_cfg.logConfig.level = 6;
  ep_cfg.uaConfig.threadCnt = 4;
  ep_cfg.medConfig.threadCnt = 4;
  ep.libInit(ep_cfg);

  auto codecs = ep.codecEnum2();
  std::sort(codecs.begin(), codecs.end(), [](pj::CodecInfo a, pj::CodecInfo b) {
    return a.priority > b.priority;
  });
  for (auto it = codecs.begin(); it < codecs.end(); ++it) {
    PJ_LOG(3, ("main", "codec[%d] %s", it->priority, it->codecId.c_str()));
  }

  // Create SIP transport. Error handling sample is shown
  try {
    pj::TransportConfig cfg;
    cfg.port = 5060;
    ep.transportCreate(PJSIP_TRANSPORT_UDP, cfg);
  } catch (pj::Error &err) {
    std::cout << err.info() << std::endl;
    return 1;
  }

  // Start the library (worker threads etc)
  ep.libStart();

  std::string line;

  SipXAccount *acc = nullptr;
  {
    // 本地账户
    pj::AccountConfig acfg;
    acfg.idUri = "sip:0.0.0.0";
    // Create the account
    acc = new SipXAccount();
    acc->create(acfg, true);
  }

  // 使用空设备
  pj::AudDevManager &aud_dev_mgr = ep.audDevManager();
  aud_dev_mgr.setNullDev();
  assert(0 == aud_dev_mgr.getDevCount());

  std::cout << "输入要呼叫的 SIP URI:" << std::endl;
  std::getline(std::cin, line);
  if (!line.empty()) {
    pj::Call *call = new SipXCall(*acc);
    pj::CallOpParam prm(true); // Use default call settings
    try {
      call->makeCall(line, prm);
    } catch (pj::Error &err) {
      std::cout << err.info() << std::endl;
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
