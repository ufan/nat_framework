#ifndef SRC_RISK_RISKSTG_H
#define SRC_RISK_RISKSTG_H

#include <unordered_map>
#include <vector>

#include "AccBase.h"
#include "RiskAcc.h"
#include "RiskInstrStg.h"
#include "RiskPrd.h"
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class RiskStg {
 public:
  bool init(const char* name, const json& j);
  bool regInstr(const char* instr_name);
  int check(int dir, int& off, double px, int vol, tSubsInstrInfo* p_subs_info);

  void onOrderTrack(const tOrderTrack* p_ord_trk);
  void onNew(int dir, int off, double px, int vol, uint32_t instr_hash,
             long nano);
  void onRtn(const tOrderTrack* p_ord_trk, const tRtnMsg* p_rtn_msg);
  void onTickPx(uint32_t instr_hash, double tick_px);
  void onSwitchDay();

  bool save(string day, bool force = false) {
    return acc_base_.save(day, force);
  }
  bool load() { return acc_base_.load(); }

  ModAcc* getModAcc() { return acc_base_.getModAcc(); }
  ModInstr* getModInstr(uint32_t instr_hash) {
    return acc_base_.getModInstr(instr_hash);
  }
  vector<ModInstr> getAllInstr() { return acc_base_.getAllInstr(); }
  vector<ModPrd> getAllPrd() { return acc_base_.getAllPrd(); }
  ModPrd* getModPrd(uint32_t prd_hash) { return acc_base_.getModPrd(prd_hash); }

  UnitAmt* getAccUnitAmt();
  UnitPnl* getAccUnitPnl();
  UnitVol* getInstrUnitVol(uint32_t instr_hash);
  UnitAmt* getInstrUnitAmt(uint32_t instr_hash);
  UnitPx* getInstrUnitPx(uint32_t instr_hash);
  UnitPnl* getInstrUnitPnl(uint32_t instr_hash);
  UnitVol* getPrdUnitVol(uint32_t prd_hash);
  UnitAmt* getPrdUnitAmt(uint32_t prd_hash);
  UnitPnl* getPrdUnitPnl(uint32_t prd_hash);

 protected:
  bool isExistRiskPrd(uint32_t prd_hash);
  bool isExistRiskInstr(uint32_t instr_hash);
  bool regRiskPrd(const char* prd_name);
  bool regRiskInstr(const char* instr_name);

 protected:
  char stg_name_[64] = {0};
  json j_conf_;

  // risk management
  unordered_map<uint32_t, RiskInstrStg> map_risk_instr_;
  unordered_map<uint32_t, RiskPrd> map_risk_prd_;
  RiskAcc risk_acc_;  // the risk account

  // base account
  AccBase acc_base_;  //  the trading account
};

#endif
