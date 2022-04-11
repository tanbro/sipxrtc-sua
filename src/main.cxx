#include <algorithm>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <pjsua2.hpp>

#include "globalvars.hxx"
#include "myaccount.hxx"
#include "mycall.hxx"

static bool running = true;

void sig_int_handler(int dummy) { running = false; }

int main(int argc, char *argv[]) {
  ep.libCreate();

  // Initialize endpoint
  pj::EpConfig ep_cfg;
  ep_cfg.logConfig.level = 6;
  ep.libInit(ep_cfg);

  // Create SIP transport. Error handling sample is shown
  pj::TransportConfig tcfg;
  tcfg.port = 5060;
  try {
    ep.transportCreate(PJSIP_TRANSPORT_UDP, tcfg);
    // ep.transportCreate(PJSIP_TRANSPORT_UDP6, tcfg);
  } catch (pj::Error &err) {
    std::cout << err.info() << std::endl;
    return 1;
  }

  // Start the library (worker threads etc)
  ep.libStart();
  std::cout << "*** PJSUA2 STARTED ***" << std::endl;

  std::string line;

  MyAccount *acc = nullptr;

  {
    // 本地账户
    pj::AccountConfig acfg;
    acfg.idUri = "sip:0.0.0.0";
    // Create the account
    MyAccount *acc = new MyAccount;
    acc->create(acfg, true);
  }

  //
  std::cout << "按 \"回车\" 键 打印音频设备列表:" << std::endl;
  std::getline(std::cin, line);

  pj::AudDevManager &aud_dev_mgr = ep.audDevManager();

  unsigned i = 0;
  auto aud_dev_vector = aud_dev_mgr.enumDev2();
  for (auto it = aud_dev_vector.begin(); it < aud_dev_vector.end(); ++it) {
    auto di = *it;
    auto did = aud_dev_mgr.lookupDev(di.driver, di.name);
    // clang-format off
    std::cout
      << "[" << i << "] "
      << "name: " << di.name << ", "
      << "driver: " << di.driver << ", "
      << "inputCount: " << di.inputCount << ", "
      << "outputCount: " << di.outputCount << ", "
      << "defaultSamplesPerSec: " << di.defaultSamplesPerSec
      << std::endl;
    // clang-format on
    ++i;
  }
  std::cout << "共发现 " << i << " 个音频设备" << std::endl;

  //////////////
  // 尝试设置:

  std::cout << "输入音频采集设备ID:" << std::endl;
  std::getline(std::cin, line);
  if (!line.empty()) {
    auto did = std::stoi(line);
    aud_dev_mgr.setCaptureDev(did);
  }

  std::cout << "输入音频播放设备ID:" << std::endl;
  std::getline(std::cin, line);
  if (!line.empty()) {
    auto did = std::stoi(line);
    aud_dev_mgr.setPlaybackDev(did);
  }


  // std::cout << "输入要呼叫的 SIP URI:" << std::endl;
  // std::getline(std::cin, line);
  // pj::Call* call = new MyCall(*acc);
  // pj::CallOpParam prm(true);  // Use default call settings
  // try {
  //   call->makeCall(line, prm);
  // } catch (pj::Error& err) {
  //   std::cout << err.info() << std::endl;
  // }

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
