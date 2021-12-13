#ifndef SRC_RISK_RISKTOP_H
#define SRC_RISK_RISKTOP_H

#include <unordered_map>
#include <vector>

#include "AccBase.h"
#include "RiskAcc.h"
#include "RiskInstrTop.h"
#include "RiskPrd.h"
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class RiskTop {
 public:
  // init
  bool init(const char* name, const json& j);
  // setup a record in base account, read in risk paramaters and configure the
  // instrument, and corresponding product
  bool regInstr(const char* instr_name);

  // check the order before ask td engine to insert a new order
  int check(int dir, int off, double px, int vol, uint32_t instr_hash);

  // update base account based on latest order tracks
  void onOrderTrack(const tOrderTrack* p_ord_trk);
  // update base account and risk account on new-issued order
  void onNew(int dir, int off, double px, int vol, uint32_t instr_hash,
             long order_ref);
  // update on cancel or traded return
  void onRtn(const tOrderTrack* p_ord_trk, const tRtnMsg* p_rtn_msg);
  // update on latest tick
  void onTickPx(uint32_t instr_hash, double tick_px);

  // save base account summary to disk
  bool save(string day, bool force = false) {
    return acc_base_.save(day, force);
  }
  // load base account summary from disk
  bool load() { return acc_base_.load(); }

  /**
   * @name Getters for base account details
   */
  vector<ModInstr> getAllInstr() { return acc_base_.getAllInstr(); }
  vector<ModPrd> getAllPrd() { return acc_base_.getAllPrd(); }
  ModInstr* getModInstr(uint32_t instr_hash) {
    return acc_base_.getModInstr(instr_hash);
  }

 protected:
  /**
   * @name For risk account configuration, internal usage
   */
  bool isExistRiskPrd(uint32_t prd_hash);
  bool isExistRiskInstr(uint32_t instr_hash);
  bool regRiskPrd(const char* prd_name);
  bool regRiskInstr(const char* instr_name);

 protected:
  char stg_name_[64] = {0};
  json j_conf_;

  // risk management
  unordered_map<uint32_t, RiskInstrTop> map_risk_instr_;
  unordered_map<uint32_t, RiskPrd> map_risk_prd_;
  RiskAcc risk_acc_;

  // base account
  AccBase acc_base_;
};

#endif
