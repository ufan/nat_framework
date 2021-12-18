/*
 * CWareHouseReader.h
 *
 *  Created on: 2018年8月2日
 *      Author: hongxu
 *
 * Comment by Yong:
 * CWareHouseReader is designed as an utility class for replaying warehouse
 * tick data fetched by dump Container. It is part of CMDHelperReplayCtpDump.
 * The decoding procedure is similar to the design of ctpdump2mmap of tool
 * package.
 *
 */

#ifndef SRC_COMMON_CWAREHOUSEREADER_H_
#define SRC_COMMON_CWAREHOUSEREADER_H_

#include <limits.h>

#include <memory>
#include <string>
#include <vector>

#include "IOCommon.h"
#include "Instrument.h"
using namespace std;

class CWareHouseFileCursor {
 public:
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

 public:
  CWareHouseFileCursor(string file);
  virtual ~CWareHouseFileCursor();
  void close();
  void getInstrumentInfo(vector<Instrument> &res);
  int next();

 public:
  int fd_ = -1;                 // file descriptor
  tFileHeadV1 file_head_;       // file header of the file
  long local_time_ = LONG_MAX;  // timestamp of current read tick
  tIOMarketData umd_;           // md format of current read tick
  string file_;                 // file managed
};

class CWareHouseReader {
 public:
  CWareHouseReader();
  virtual ~CWareHouseReader();

  void openFile(string file);

  void switchDay();

  int merge(tIOMarketData &data_rtn);

  void loadFileList(string file_pattern, string start_day, string end_day);

  void loadFiles(vector<string> file_patterns, string start_day,
                 string end_day);

  virtual void onSwitchDay(string instrInfo) {}

  const UnitedMarketData *readTick();

  void setReadPos(long nano);

  void clear() {
    files_.clear();
    cur_trading_day_.clear();
  }

  void doSetTimer(bool flag) { do_set_time_ = flag; }

 protected:
  tIOMarketData tick_;      // current tick data read
  long local_time_ = 0;     // current tick time
  string cur_trading_day_;  // current trading day

  vector<unique_ptr<CWareHouseFileCursor>> files_;  // list of file readers
  vector<string> file_patterns_;  // patterns specifying data source file range
  string start_day_;              // date range specifying data source
  string end_day_;

  bool do_set_time_ = false;  // clock time alignment or not
};

#endif /* SRC_COMMON_CWAREHOUSEREADER_H_ */
