#include <fstream>
#include <string>
#include <vector>

#include "CCTPMD.h"
#include "CCTPTD.h"
#include "DailyInfoMgr.h"
#include "FileMgr.h"
#include "Logger.h"
#include "Structure.h"
#include "json.hpp"
#include "signal.h"
#include "stdio.h"
#include "string.h"
#include "unistd.h"

using json = nlohmann::json;
using namespace std;

CCTPTD* p_td = nullptr;
CCTPMD* p_md = nullptr;

bool run(string config) {
  // step 1: read and parse configure file
  ifstream in(config);
  if (!in) {
    ALERT("read config file %s err.\n", config.c_str());
    return false;
  }
  string content((std::istreambuf_iterator<char>(in)),
                 std::istreambuf_iterator<char>());
  in.close();
  json j_conf = json::parse(content);

  // step 2: initialize logger
  initLogger(j_conf["log4cplus"]);
  if (!FileMgr::init(j_conf)) {
    ALERT("can't init FileMgr.\n");
    return false;
  }

  // step 3: read in the CTP front information
  json j = j_conf[j_conf["ctp_name"].get<string>()];
  int sleep_sec = j["timeout"].get<int>();

  // step 4: init Trader and enter into Trader event loop
  p_td = new CCTPTD;
  for (; p_td->status < 5;) {
    switch (p_td->status) {
      case 0:                  // initial state
        if (!p_td->init(j)) {  // try to connect to the trading front
          ALERT("can't init ctp td.\n");
        }
        sleep(5);
        break;
      case 1:                   // front connected
        if (!p_td->login(j)) {  // try log into the front
          ALERT("can't login ctp td.\n");
        }
        sleep(2);
        break;
      case 2:                     // login succeed
        if (!p_td->confirm(j)) {  // confirm the settlement before query
          ALERT("can't confirm settlement info.\n");
        }
        sleep(4);
        break;
      case 3:                          // settlement confirmed
        if (!p_td->qryInstrument()) {  // query list of instruments traded
                                       // today
          ALERT("can't query instruments.\n");
        }
        sleep(2);
        break;
      case 4:  // all available instruments returned and recorded
        if (!p_td->release()) {  // release TradeApi
          ALERT("can't release ctp td.\n");
        }
        break;
      default:
        break;
    }
  }

  // step 5: write header (i.e. the basic data of all instruments) into both
  // binary and JSON files
  if (!FileMgr::regInstrument(p_td->trading_date, p_td->vec_instr)) {
    ALERT("can't register instruments in FileMgr.\n");
    return false;
  }

  if (!DailyInfoMgr::init(j_conf, p_td->trading_date)) {
    ALERT("can't init DailyInfoMgr.\n");
    return false;
  }

  if (!DailyInfoMgr::write()) {
    ALERT("can't write daily_info.\n");
    return false;
  }

  // step 6: init MD and enter into the MD event loop
  p_md = new CCTPMD;
  for (; p_md->status < 5;) {
    if (!p_md->isInTradingTime()) {  // if not trading now, exit program
      p_md->status = 4;
    }

    switch (p_md->status) {
      case 0:
        if (!p_md->init(j)) {  // init and try to connect Thost
          ALERT("can't init ctp md\n");
        }
        sleep(2);
        break;
      case 1:
        if (!p_md->login(j)) {  // init success and try to login Thost
          ALERT("can't login ctp md\n");
        }
        sleep(2);
        break;
      case 2:
        if (!p_md->regInstrument(
                p_td->vec_instr)) {  // login success and subscribe all
                                     // available instruments
          ALERT("can't reg instr\n");
        }
        sleep(2);
        break;
      case 3:  // infinite loop and just receive and write the tick data into
               // files. Only to be interrupted by the singal_handler like
               // Ctrl-C
        sleep(sleep_sec);
        break;
      case 4:
        if (!p_md->release()) {
          ALERT("can't release ctp md.\n");
        }
        break;
      default:
        break;
    }
  }

  // step 7: if exit the MD event loop normally, the do the release work here
  if (!FileMgr::release()) {
    ALERT("failed in FileMgr release.\n");
    return false;
  }

  if (!DailyInfoMgr::release()) {
    ALERT("failed in DailyInfoMgr release.\n");
    return false;
  }

  return true;
}

void signal_handler(int sig) {
  ENGLOG("pid %d recv signal %d.\n", getpid(), sig);
  if (p_td) {
    p_td->release();
    delete p_td;
    p_td = nullptr;
  }

  if (p_md) {
    p_md->release();
    delete p_md;
    p_md = nullptr;
  }

  if (!FileMgr::release()) {
    ALERT("fail in FileMgr release.\n");
  }

  if (!DailyInfoMgr::release()) {
    ALERT("failed in DailyInfoMgr release.\n");
  }

  ENGLOG("main func ready to exit normally.\n");
  exit(EXIT_SUCCESS);
}

int main(int argc, char* argv[]) {
  if (argc < 2) {
    printf("Usage: bin/CTPDumper conf/dump.json\n");
    exit(EXIT_FAILURE);
  } else {
    for (int i = 0; i < argc; ++i) {
      ENGLOG("argv[%d]: %s\n", i, argv[i]);
    }
  }

  signal(SIGTERM, signal_handler);
  signal(SIGINT, signal_handler);
  signal(SIGHUP, signal_handler);
  signal(SIGQUIT, signal_handler);
  signal(SIGKILL, signal_handler);

  if (!run(argv[1])) {
    ALERT("run failed!\n");
  }

  ENGLOG("main func end.\n");
  sleep(5);
  return 0;
}
