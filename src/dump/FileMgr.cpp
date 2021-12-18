#include "FileMgr.h"

#include "Logger.h"
#include "Structure.h"
#include "string.h"
#include "utils.h"

extern int errno;
const int FILE_HEAD_VER = 1;
const int DUMP_MD_HEAD_VER = 1;
const UnitedMarketDataType UMD_TYPE = UnitedMarketDataType::CTP_v638;

FILE* FileMgr::pf_all = nullptr;
unordered_map<string, string> FileMgr::map_instr_prd;
unordered_map<string, vector<Instrument>> FileMgr::map_prd_instr;
unordered_map<string, string> FileMgr::map_prd_exch;
unordered_map<string, FILE*> FileMgr::map_prd_file;
char FileMgr::warehouse[256] = {0};

bool FileMgr::init(json& j) {
  ENGLOG("FileMgr init.\n");
  strcpy(warehouse, j["warehouse_path"].get<string>().c_str());
  return createPath(warehouse);
}

/**
 * @brief Close all the files and release the resource.
 *        TODO: pf_all is not released properly
 */
bool FileMgr::release() {
  ENGLOG("FileMgr release.\n");
  for (auto it = map_prd_file.begin(); it != map_prd_file.end(); ++it) {
    if (it->second) {
      fclose(it->second);
      it->second = nullptr;
    }
  }
  return true;
}

/**
 * @brief Fill in the mappings and write the instrument information into files.
 *
 * @details These information is written as the header of the file. If the file
 * already exists and contains the header already, then just open the file in
 * appending mode.
 */
bool FileMgr::regInstrument(const char* trading_date,
                            vector<Instrument>& vec_instr) {
  map_instr_prd.clear();
  map_prd_file.clear();

  for (Instrument instr : vec_instr) {
    map_instr_prd[instr.instr_str] = instr.prd_str;
    map_prd_instr[instr.prd_str].emplace_back(instr);
    map_prd_exch[instr.prd_str] = instr.exch_str;
  }

  for (auto it = map_prd_instr.begin(); it != map_prd_instr.end(); ++it) {
    // Group 1 based on exch, prd and trade date
    char dirname[256] = {0};
    snprintf(dirname, sizeof(dirname), "%s/China/%s/%s", warehouse,
             map_prd_exch[it->first].c_str(), it->first.c_str());
    if (!createPath(dirname)) {
      ALERT("can't create path %s.\n", dirname);
      return false;
    }

    char filename[256] = {0};
    snprintf(filename, sizeof(filename), "%s/CTP_%s_%s_%s.data", dirname,
             trading_date, map_prd_exch[it->first].c_str(), it->first.c_str());
    FILE* pf = fopen(filename, "ab");  // open or create in append mode
    if (pf == nullptr) {
      ALERT("can't open file %s, errno:%d, errmsg:%s\n", filename, errno,
            strerror(errno));
      return false;
    }
    if (fseek(pf, 0, SEEK_END) == -1) {  // go to eof
      ALERT("can't seek file %s, errno:%d, errmsg:%s\n", filename, errno,
            strerror(errno));
      return false;
    }
    long pos = ftell(pf);
    if (pos == 0) {  // if newly-created, write the header and the base info; if
                     // existing, do nothing
      int instr_cnt = it->second.size();
      fwrite(&FILE_HEAD_VER, sizeof(int), 1, pf);
      fwrite(&DUMP_MD_HEAD_VER, sizeof(int), 1, pf);
      fwrite(&UMD_TYPE, sizeof(UnitedMarketDataType), 1, pf);
      fwrite("", 16, 1, pf);
      fwrite(trading_date, 16, 1, pf);
      fwrite("", 32, 1, pf);
      fwrite(&instr_cnt, sizeof(int), 1, pf);
      for (int i = 0; i < instr_cnt; ++i) {
        fwrite(&it->second[i], sizeof(Instrument), 1, pf);
      }
    }

    // keep the file descriptor for tick data writing
    map_prd_file[it->first] = pf;
  }

  // Group 2: based on trade date
  char dirname[256] = {0};
  snprintf(dirname, sizeof(dirname), "%s/China/All", warehouse);
  if (!createPath(dirname)) {
    ALERT("can't create path %s.\n", dirname);
    return false;
  }

  char filename[256] = {0};
  snprintf(filename, sizeof(filename), "%s/CTP_%s_All.data", dirname,
           trading_date);
  FILE* pf = fopen(filename, "ab");
  if (pf == nullptr) {
    ALERT("can't open file %s, errno:%d, errmsg:%s\n", filename, errno,
          strerror(errno));
    return false;
  }
  if (fseek(pf, 0, SEEK_END) == -1) {
    ALERT("can't seek file %s, errno:%d, errmsg:%s\n", filename, errno,
          strerror(errno));
    return false;
  }
  long pos = ftell(pf);
  if (pos == 0) {
    int instr_cnt = vec_instr.size();
    fwrite(&FILE_HEAD_VER, sizeof(int), 1, pf);
    fwrite(&DUMP_MD_HEAD_VER, sizeof(int), 1, pf);
    fwrite(&UMD_TYPE, sizeof(UnitedMarketDataType), 1, pf);
    fwrite("", 16, 1, pf);
    fwrite(trading_date, 16, 1, pf);
    fwrite("", 32, 1, pf);
    fwrite(&instr_cnt, sizeof(int), 1, pf);
    for (int i = 0; i < instr_cnt; ++i) {
      fwrite(&vec_instr[i], sizeof(Instrument), 1, pf);
    }
  }
  pf_all = pf;

  return true;
}

/**
 * @brief Return the file handler corresponding the name of the instrument
 * contract.
 */
FILE* FileMgr::getFilePtr(const char* instr_str) {
  string prd_str = map_instr_prd[instr_str];
  return map_prd_file[prd_str];
}
