/*
 * ctpdump2mmap.cpp
 *
 *  Created on: Jun 20, 2018
 *      Author: hongxu
 *
 * Comment by Yong:
 * ctpdump2mmap will merge a collection of warehouse files provided by the user
 * into the md engine mmap page file in an output directory specified by user.
 * The warehouse files are produced by CTPDumper from dump Container.
 * The mmap file has the same format as the real-time md engine's io page file.
 * Base info is written first for each day. The tick data are sorted by
 * timestamp.
 */
#include <error.h>
#include <fcntl.h>
#include <glob.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#include <iostream>

#include "ATStructure.h"
#include "CRawIOWriter.h"
#include "CTradeBaseInfo.h"
#include "IOCommon.h"
#include "Instrument.h"
#include "MurmurHash2.h"
#include "OldUnitedMarketData.h"
#include "utils.h"
using namespace std;

#pragma pack(push, 1)
struct tFileHeadV1 {
  int file_ver;
  int md_head_ver;
  int md_body_ver;
  char system_date[16];
  char trading_date[16];
  char system_day_night[32];
  int inst_cnt;
  Instrument inst_arr[0];
};
#pragma pack(pop)

class CFileCursor {
 public:
  CFileCursor(string file) {
    file_ = file;
    fd_ = open(file.c_str(), O_RDONLY | O_CLOEXEC);
    if (fd_ < 0) {
      perror("open file err");
      throw std::runtime_error("open file err.");
    }
    if (sizeof(file_head_) != read(fd_, &file_head_, sizeof(file_head_))) {
      cerr << file << " format err" << endl;
      throw std::runtime_error("file format err.");
    }
  }
  ~CFileCursor() {}
  void close() {
    if (fd_ >= 0) {
      ::close(fd_);
      fd_ = -1;
    }
  }
  void getInstrumentInfo(vector<Instrument> &res) {
    Instrument info;
    for (int i = 0; i < file_head_.inst_cnt; ++i) {
      int iret = read(fd_, &info, sizeof(info));
      if (iret < sizeof(info)) {
        throw std::runtime_error("parse instrument info format err.");
      }
      res.push_back(info);
    }
  }
  int next() {
    MarketDataHead md_head;
    int iread = read(fd_, &md_head, sizeof(md_head));
    if (iread == 0)
      return 0;
    else if (sizeof(md_head) != iread)
      return -1;
    local_time_ = md_head.local_time;

    CThostFtdcDepthMarketDataField_v638 data;
    iread = read(fd_, &data, sizeof(data));
    if (iread == 0)
      return 0;
    else if (sizeof(data) != iread)
      return -1;

    UnitedMarketData &umd = umd_.market_data;
    strncpy(umd.instr_str, data.InstrumentID, sizeof(umd.instr_str));
    umd.instr_str[sizeof(umd.instr_str) - 1] = '\0';
    umd.instr_hash = INSTR_NAME_TO_HASH(umd.instr_str);
    umd.last_px = data.LastPrice;
    umd.cum_vol = data.Volume;
    umd.cum_turnover = data.Turnover;
    umd.avg_px = data.AveragePrice;
    umd.ask_px = data.AskPrice1;
    umd.bid_px = data.BidPrice1;
    umd.ask_vol = data.AskVolume1;
    umd.bid_vol = data.BidVolume1;
    umd.open_interest = data.OpenInterest;
    umd.exch_time =
        (local_time_ + 8 * 3600000000000L) / 86400000000000L * 86400000000000L +
        getSecondsFromClockStr(data.UpdateTime) * 1000000000L +
        data.UpdateMillisec * 1000000L - 8 * 3600000000000L;
    if (umd.exch_time > local_time_ + 2 * 3600000000000L)
      umd.exch_time -= 86400000000000L;
    else if (umd.exch_time < local_time_ - 20 * 3600000000000L)
      umd.exch_time += 86400000000000L;
    return iread;
  }

 public:
  int fd_ = -1;
  tFileHeadV1 file_head_;
  long local_time_ = LONG_MAX;
  tIOMarketData umd_;
  string file_;
};

class CMerger {
 public:
  CMerger(string dir) {
    if (dir.empty()) {
      throw std::runtime_error("dst dir empty!");
    }
    if (dir.back() != '/') dir.push_back('/');
    if (not writer_.init(dir + "md")) {
      throw std::runtime_error("init writer err.");
    }
  }
  ~CMerger() {}

  void openFile(string file) { files_.emplace_back(file); }

  void switchDay() {
    // 1. find the earliest day in the remaining files
    if (files_.empty()) return;
    cur_trading_day_ = files_[0].file_head_.trading_date;
    for (int i = 1; i < files_.size(); ++i) {
      if (cur_trading_day_ > files_[i].file_head_.trading_date) {
        cur_trading_day_ = files_[i].file_head_.trading_date;
      }
    }
    LOG_INFO("process trading day:%s", cur_trading_day_.c_str());
    CTradeBaseInfo::trading_day_ = cur_trading_day_;
    CTradeBaseInfo::instr_info_.clear();
    vector<Instrument> instrs;

    // 2. find the earliest tick time in the file collection
    // reteive base info of the earliest trade day in the same time
    long min_nano = LONG_MAX;
    for (int i = 0; i < files_.size(); ++i) {
      if (cur_trading_day_ == files_[i].file_head_.trading_date) {
        files_[i].getInstrumentInfo(instrs);
        int ret = files_[i].next();
        if (ret <= 0) {
          if (ret < 0) {
            LOG_ERR("detect file format err.");
          }
          files_[i].close();
          files_.erase(files_.begin() + i);
          i--;
        } else if (min_nano > files_[i].local_time_) {
          min_nano = files_[i].local_time_;
        }
      }
    }

    // 3. init trade base repo
    for (auto &i : instrs) {
      uint32_t hash = INSTR_NAME_TO_HASH(i.instr_str);
      if (CTradeBaseInfo::instr_info_.count(hash) == 0) {
        tInstrumentInfo &info = CTradeBaseInfo::instr_info_[hash];
        info.instr_hash = hash;
        strncpy(info.instr, i.instr_str, sizeof(info.instr));
        info.instr[sizeof(info.instr) - 1] = '\0';
        info.exch = exchangeStr2int(i.exch_str);
        strncpy(info.product, i.prd_str, sizeof(info.product));
        info.product[sizeof(info.product) - 1] = '\0';
        info.product_hash = INSTR_NAME_TO_HASH(info.product);
        info.vol_multiple = i.volume_multiple;
        info.tick_price = i.price_tick;
        strncpy(info.expire_date, i.expire_date, sizeof(info.expire_date));
        info.expire_date[sizeof(info.expire_date) - 1] = '\0';
      }
    }

    // 4. write trade base info into mmap file
    string data = CTradeBaseInfo::toSysIOStruct(0, 0, 0);
    CTimer::instance().setTime(min_nano - 10000000000L);
    writer_.write(data.data(), data.size());
  }

  // sort the tick data of the same day based on timestamp
  // If no more data for the current day, switch to next earliest day
  void merge() {
    long local_time = LONG_MAX;
    int idx = -1;
    for (int i = 0; i < files_.size(); ++i) {
      if (cur_trading_day_ == files_[i].file_head_.trading_date) {
        if (local_time > files_[i].local_time_) {
          local_time = files_[i].local_time_;
          idx = i;
        }
      }
    }

    if (idx >= 0) {
      // new tick data available
      CTimer::instance().setTime(local_time);
      writer_.write(&(files_[idx].umd_), sizeof(tIOMarketData));

      // try to read next tick
      int ret = files_[idx].next();
      if (ret <= 0) {
        if (ret < 0) {
          LOG_ERR("detect file format err.");
        }

        // end of file reached, close remove this file out of the collection
        files_[idx].close();
        files_.erase(files_.begin() + idx);
      }
    } else {
      // no more tick data for the current day
      switchDay();
    }
  }

  void run() {
    // Init state: switch to the earliest day
    switchDay();

    while (files_.size()) {
      merge();
    }
  }

 public:
  CRawIOWriter writer_;
  vector<CFileCursor> files_;
  string cur_trading_day_;
};

int main(int argc, char *argv[]) {
  if (argc < 3) {
    printf("Usage: %s dst_dir ctpfiles...\n", argv[0]);
    return -1;
  }

  initLogger("./logger.cnf");

  CMerger m(argv[1]);
  for (int i = 2; i < argc; i++) {
    glob_t buf;
    glob(argv[i], GLOB_NOSORT, NULL, &buf);
    for (int j = 0; j < buf.gl_pathc; j++) {
      m.openFile(buf.gl_pathv[j]);
    }
    globfree(&buf);
  }

  m.run();
  cout << "done!" << endl;
  return 0;
}
