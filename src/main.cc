#include <algorithm>
#include <csignal>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <iomanip>
#include <iostream>
#include <thread>

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

void sig_int_handler(int dummy) {
  LOG(INFO) << "sig_int_handler: " << hex << this_thread::get_id();
  running = false;
}

int main(int argc, char *argv[]) {
  gflags::SetVersionString(getVersionString());
  gflags::ParseCommandLineFlags(&argc, &argv, true);
  google::InitGoogleLogging(argv[0]);

  LOG(WARNING) << endl
               << "================ startup ================" << endl
               << argv[0] << endl
               << "[" << hex << this_thread::get_id() << "]" << endl
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
    if (FLAGS_sip_log_level >= 0) {
      cfg.logConfig.level = FLAGS_sip_log_level;
    }
    if (!FLAGS_sip_log_file.empty()) {
      cfg.logConfig.filename = FLAGS_sip_log_file;
    }
    if (FLAGS_sip_console_level >= 0) {
      cfg.logConfig.consoleLevel = FLAGS_sip_console_level;
    }
    VLOG(1) << ">>> pj::Endpoint::libInit()";
    ep.libInit(cfg);
    VLOG(1) << "<<< pj::Endpoint::libInit()";
  }

  if (FLAGS_list_codecs) {
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
    cfg.port = FLAGS_sip_port;
    if (cfg.port > 0) {
      cfg.portRange = FLAGS_sip_port_range;
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
    cfg.mediaConfig.transportConfig.port = FLAGS_rtp_port;
    cfg.mediaConfig.transportConfig.portRange = FLAGS_rtp_port_range;
    cfg.mediaConfig.srtpUse = (pjmedia_srtp_use)FLAGS_srtp_use;
    // acc_cfg.idUri = "sip:8007@192.168.2.202";
    // acc_cfg.regConfig.registrarUri = "sip:192.168.2.202";
    // acc_cfg.sipConfig.authCreds.push_back(
    //     pj::AuthCredInfo("digest", "*", "8007", 0, "hesong"));
    // Create the account
    account.create(cfg, true);
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
    int nfds = poller.refillFds();
    if (nfds > 0) {
      poller.performOnce(1000);
      // auto performed = poller.performOnce(1000);
      // if (!performed) {
      //   this_thread::sleep_for(chrono::milliseconds(100));
      // }
    } else {
      this_thread::sleep_for(chrono::milliseconds(100));
    }
  }

  //////////////////

  // Delete the account. This will unregister from server]
  LOG(INFO) << "关闭所有 SIP 呼叫";
  ep.hangupAllCalls();
  LOG(INFO) << "释放 SIP Stack";
  ep.libDestroy();

  //////////////////
  return 0;
}
