#ifndef SRC_ACCOUNT_ACCBASE_H
#define SRC_ACCOUNT_ACCBASE_H

#include <string>
#include <unordered_map>
#include <vector>

#include "ATStructure.h"
#include "ModAcc.h"
#include "ModInstr.h"
#include "ModPrd.h"
using namespace std;

class AccBase {
 public:
  // init this account with the meta info from json config
  bool init(const char* name, const json& j);
  // add a record for the new instrument into this account
  bool regInstr(const char* instr_name);

  // load the initial account data from disk file
  bool load();
  // save the account data to disk file
  bool save(string day, bool force = false);

  /**
   * @name Callbacks by the trading strategy to update the account info
   */
  void onSwitchDay();
  void onNew(int dir, int off, double px, int vol, uint32_t instr_hash);
  void onRtn(const tOrderTrack* p_ord_trk, const tRtnMsg* p_rtn_msg);
  void onTickPx(uint32_t instr_hash, double tick_px);
  void onOrderTrack(const tOrderTrack* p_ord_trk);

  /**
   * @name Get the summary of this account, product and instruments
   */
  ModAcc* getModAcc();
  ModInstr* getModInstr(uint32_t instr_hash);
  ModPrd* getModPrd(uint32_t prd_hash);
  vector<ModInstr> getAllInstr();
  vector<ModPrd> getAllPrd();

  /**
   * @name Get the specific field of this account, products and instruments
   */
  UnitAmt* getAccUnitAmt();
  UnitPnl* getAccUnitPnl();
  UnitVol* getInstrUnitVol(uint32_t instr_hash);
  UnitAmt* getInstrUnitAmt(uint32_t instr_hash);
  UnitPx* getInstrUnitPx(uint32_t instr_hash);
  UnitPnl* getInstrUnitPnl(uint32_t instr_hash);
  UnitVol* getPrdUnitVol(uint32_t prd_hash);
  UnitAmt* getPrdUnitAmt(uint32_t prd_hash);
  UnitPnl* getPrdUnitPnl(uint32_t prd_hash);

  // the collection of traded instruments detail
  unordered_map<uint32_t, ModInstr> map_instr_;
  // the collection of product summary of the traded instruments
  unordered_map<uint32_t, ModPrd> map_prd_;
  // the summary of this trading account
  ModAcc mod_acc_;

 protected:
  bool isExistModPrd(uint32_t prd_hash);
  bool isExistModInstr(uint32_t instr_hash);
  bool regModPrd(const char* prd_name);
  bool regModInstr(const char* instr_name);

  // get the path string for in/out summary file
  bool getJsonFilePath(int t, string day, string& path);

  json to_json();  // output instr, product, account summary to json format
  bool from_json(json& j);  // get the above info from json content

  json j_conf_;              // configuration
  char acc_name_[64] = {0};  // unique name of this account (for end user)
};

#endif
