#include <algorithm>
#include <iomanip>
#include <iostream>
#include <ostream>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <iostream>
#include <sstream>

#include <pjsua2.hpp>

#include <gflags/gflags.h>
#include <glog/logging.h>

#include "ArgsFlags.hh"
#include "SipXAccount.hh"
#include "SipXCall.hh"
#include "global.hh"
#include "version.hh"

using namespace std;
using namespace sipxsua;

static bool running = true;

void sig_int_handler(int dummy) { running = false; }

int main(int argc, char *argv[]) {
  gflags::SetVersionString(getVersionString());
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  LOG(WARNING) << endl
               << "================ startup ================" << endl
               << argv[0] << endl
               << "  version: " << getVersionString() << endl
               << "^^^^^^^^^^^^^^^^ startup ^^^^^^^^^^^^^^^^" << endl;

  LOG(INFO) << "Create SIP library";
  VLOG(1) << ">>> pj::Endpoint::libCreate()";
  ep.libCreate();
  VLOG(1) << "<<< pj::Endpoint::libCreate()";

  // Initialize endpoint
  LOG(INFO) << "Initialize SIP endpoint";
  {
    pj::EpConfig cfg;
    if (FLAGS_pj_log_level >= 0) {
      cfg.logConfig.level = FLAGS_pj_log_level;
    }
    if (!FLAGS_pj_log_file.empty()) {
      cfg.logConfig.filename = FLAGS_pj_log_file;
    }
    if (FLAGS_pj_log_console_level >= 0) {
      cfg.logConfig.consoleLevel = FLAGS_pj_log_console_level;
    }
    VLOG(1) << ">>> pj::Endpoint::libInit()";
    ep.libInit(cfg);
    VLOG(1) << "<<< pj::Endpoint::libInit()";
  }

  if (FLAGS_pj_list_codecs) {
    auto codecs = ep.codecEnum2();
    sort(codecs.begin(), codecs.end(), [](pj::CodecInfo a, pj::CodecInfo b) {
      return a.priority > b.priority;
    });
    cout << endl;
    cout << "======== "
         << "================ "
         << "=================" << endl;
    cout << "Priority "
         << "Id               "
         << "Description" << endl;
    cout << "======== "
         << "================ "
         << "=================" << endl;
    for (auto it = codecs.begin(); it < codecs.end(); ++it) {
      cout << setw(8) << right << (unsigned)it->priority;
      cout << " " << setw(16) << left << it->codecId;
      cout << " " << it->desc;
      cout << endl;
    }
    cout << "======== "
         << "================ "
         << "=================" << endl;
    cout << endl;
    return 0;
  }

  // Create SIP transport. Error handling sample is shown
  LOG(INFO) << "Create SIP transport";
  {
    pj::TransportConfig cfg;
    cfg.port = FLAGS_pj_sip_port;
    if (cfg.port > 0) {
      cfg.portRange = FLAGS_pj_sip_port_range;
    }
    VLOG(1) << ">>> pj::Endpoint::transportCreate("
            << "boundAddress=\"" << cfg.boundAddress << "\", "
            << "port=" << cfg.port << ",  "
            << "portRange=" << cfg.portRange << ", "
            << "publicAddress=\"" << cfg.publicAddress << "\""
            << ")";
    pj::TransportId tid;
    CHECK_LE(0, (tid = ep.transportCreate(PJSIP_TRANSPORT_UDP, cfg)))
        << "<<< pj::Endpoint::transportCreate() failed";
    {
      auto ti = ep.transportGetInfo(tid);
      VLOG(1) << "<<< pj::Endpoint::transportCreate() succeed: " << ti.info;
    }
  }

  // Start the library (worker threads etc)
  LOG(INFO) << "Start SIP library";
  VLOG(1) << ">>> pj::Endpoint::libStart()";
  ep.libStart();
  VLOG(1) << "<<< pj::Endpoint::libStart()";

  string line;

  LOG(INFO) << "Create default local SIP account";
  {
    // 本地账户
    pj::AccountConfig cfg;
    cfg.idUri = "sip:0.0.0.0";
    cfg.mediaConfig.transportConfig.port = FLAGS_pj_rtp_port;
    cfg.mediaConfig.transportConfig.portRange = FLAGS_pj_rtp_port_range;
    cfg.mediaConfig.srtpUse = (pjmedia_srtp_use)FLAGS_pj_srtp_use;
    // acc_cfg.idUri = "sip:8007@192.168.2.202";
    // acc_cfg.regConfig.registrarUri = "sip:192.168.2.202";
    // acc_cfg.sipConfig.authCreds.push_back(
    //     pj::AuthCredInfo("digest", "*", "8007", 0, "hesong"));
    // Create the account
    sipAcc.create(cfg, true);
  }

  // 使用空设备
  pj::AudDevManager &aud_dev_mgr = ep.audDevManager();
  aud_dev_mgr.setNullDev();
  CHECK_EQ(0, aud_dev_mgr.getDevCount()) << "audDevManager::setNullDev() 失败";

  // cout << "输入要呼叫的 SIP URI:" << endl;
  // getline(cin, line);
  // if (!line.empty()) {
  //   pj::Call *call = new SipXCall(sipAcc);
  //   pj::CallOpParam prm(true); // Use default call settings
  //   try {
  //     call->makeCall(line, prm);
  //   } catch (pj::Error &err) {
  //     cout << err.info() << endl;
  //     LOG(ERROR) << "makeCall 失败: (" << err.status << " " << err.reason
  //                << ") " << err.info();
  //   }
  // }

  //////////////
  printf("ctrl-c 退出\n");
  signal(SIGINT, sig_int_handler);
  while (running) {
    sleep(1);
  }
  //////////////////

  // Delete the account. This will unregister from server
  ep.hangupAllCalls();
  ep.libDestroy();

  //////////////////
  return 0;
}
