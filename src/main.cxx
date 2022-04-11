#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <algorithm>

#include <iostream>
#include <pjsua2.hpp>

#include "globalvars.hxx"
#include "myaccount.hxx"
#include "mycall.hxx"

static bool running = true;

void sig_int_handler(int dummy) {
  running = false;
}

int main(int argc, char* argv[]) {
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
  } catch (pj::Error& err) {
    std::cout << err.info() << std::endl;
    return 1;
  }

  // Start the library (worker threads etc)
  ep.libStart();
  std::cout << "*** PJSUA2 STARTED ***" << std::endl;

  std::string line;

  MyAccount* acc = nullptr;

  {
    // 本地账户
    pj::AccountConfig acfg;
    acfg.idUri = "sip:0.0.0.0";
    // Create the account
    MyAccount* acc = new MyAccount;
    acc->create(acfg, true);
  }

  // std::cout << "是否注册账号 sip:8007@192.168.2.202 [y/n]" << std::endl;
  // std::getline(std::cin, line);
  // if (line == "y") {
  //   // Configure an AccountConfig
  //   pj::AccountConfig acfg;
  //   acfg.idUri = "sip:8007@192.168.2.202";
  //   acfg.regConfig.registrarUri = "sip:192.168.2.202";
  //   pj::AuthCredInfo cred("digest", "*", "8007", 0, "hesong");
  //   acfg.sipConfig.authCreds.push_back(cred);

  //   // Create the account
  //   MyAccount* acc = new MyAccount;
  //   acc->create(acfg);
  // }

  //
  pj::AudDevManager& adev_mgr = ep.audDevManager();


  // auto dev_count = adev_mgr.getDevCount();
  // auto c_did = adev_mgr.getCaptureDev();
  // auto p_did = adev_mgr.getPlaybackDev();
  // auto c_di = adev_mgr.getDevInfo(c_did);
  // auto p_di = adev_mgr.getDevInfo(p_did);
  // std::cout << "Audio 设备数量: " << dev_count << std::endl;
  // std::cout << "Audio 当前回放设备: [" << p_did << "]" << c_di.driver << ". "
  //           << c_di.name << std::endl;
  // std::cout << "Audio 当前采集设备: [" << p_did << "]" << p_di.driver << ". "
  //           << p_di.name << std::endl;


  int pulse_dev_id = -1;
  auto di_vct = adev_mgr.enumDev2();
  int i = 0;
  for (auto it = di_vct.begin(); it < di_vct.end(); ++it) {
    auto di = *it;
    auto did = adev_mgr.lookupDev(di.driver, di.name);
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
    if ((di.name == "pulse") && (di.driver == "PA")) {
      if (pulse_dev_id >= 0) {
        std::cerr << "! PortAudio 发现了多个 PulseAudio 设备" << std::endl;
      }
      pulse_dev_id = i;
    }
    ++i;
  }

  //////////////
  // 尝试设置:

  // 在 PortAudio 上寻找 PulseAudio 设备！
  // auto did = adev_mgr.lookupDev("pulse", "PA");
  // adev_mgr.setCaptureDev(pulse_dev_id);
  // adev_mgr.setPlaybackDev(pulse_dev_id);

  std::cout << "输入音频采集设备ID:" << std::endl;
  std::getline(std::cin, line);
  if (!line.empty()) {
    auto did = std::stoi(line);
    adev_mgr.setCaptureDev(did);
  }

  std::cout << "输入音频播放设备ID:" << std::endl;
  std::getline(std::cin, line);
  if (!line.empty()) {
    auto did = std::stoi(line);
    adev_mgr.setPlaybackDev(did);
  }

  // std::cout << "是否设置音频采集设备为 ALSA " << default_capture_device_name
  //           << " ? [y/n]" << std::endl;
  // std::getline(std::cin, line);
  // {
  //   auto did = dev_mgr.lookupDev("ALSA", default_capture_device_name);
  //   if (did < 0) {
  //     throw new std::runtime_error("找不到设备");
  //   }
  //   dev_mgr.setCaptureDev(did);
  // }

  //////////////
  printf("ctrl-c 退出\n");
  signal(SIGINT, sig_int_handler);
  while (running) {
    sleep(1);
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
