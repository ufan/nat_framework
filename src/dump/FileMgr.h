/**
 * @file      FileMgr.h
 * @brief     Header of FileMgr
 * @date      Tue Dec  7 20:57:58 2021
 * @author    Yong
 * @copyright BSD-3-Clause
 *
 * FileMgr is responsible for sorting and writing the instrument basic info and
 * the tick data into binary files, which is called as warehouse data.
 *
 * Two copies of the data is saved onto disk based on trading date:
 * 1. Categorized based on exchange, product and trading date.
 *    Directory name pattern: $warehouse_dir$/China/$exchange$/$product$
 *    File name pattern: CTP_$trading_date$_$exchange$_$product$.data
 * 2. Aggregation of all instruments of the same trading day into a single file
 *    Directory name pattern: $warehouse_dir$/China/All
 *    File name pattern: CTP_$trading_date$_All.data
 *
 * File format: see note.org
 */

#ifndef DUMP_FILEMGR_H
#define DUMP_FILEMGR_H

#include <string>
#include <unordered_map>
#include <vector>

#include "Structure.h"
#include "json.hpp"

using json = nlohmann::json;
using namespace std;

class FileMgr {
 public:
  static bool init(json& j);
  static bool release();
  static bool regInstrument(const char* trading_date,
                            vector<Instrument>& vec_instr);
  static FILE* getFilePtr(const char* instr_str);

  static unordered_map<string, string> map_instr_prd;  // instrument -> product
  static unordered_map<string, vector<Instrument>>
      map_prd_instr;  // product -> list of instruments of the same product
                      // but different expiration date
  static unordered_map<string, string> map_prd_exch;  // product -> exchange
  static unordered_map<string, FILE*> map_prd_file;   // product -> file handler
  static FILE* pf_all;         // handler of the file containing all instruments
  static char warehouse[256];  // directory as the ware house
};

#endif
