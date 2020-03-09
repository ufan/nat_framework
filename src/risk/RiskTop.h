#ifndef SRC_RISK_RISKTOP_H
#define SRC_RISK_RISKTOP_H

#include "RiskInstrTop.h"
#include "RiskPrd.h"
#include "RiskAcc.h"
#include "AccBase.h"
#include <vector>
#include <unordered_map>
#include "json.hpp"
using json = nlohmann::json;
using namespace std;

class RiskTop
{
public:
	bool init(const char* name, const json& j);
	bool regInstr(const char* instr_name);
	int check(int dir, int off, double px, int vol, uint32_t instr_hash);
	
	void onOrderTrack(const tOrderTrack* p_ord_trk);
	void onNew(int dir, int off, double px, int vol, uint32_t instr_hash, long order_ref);
	void onRtn(const tOrderTrack* p_ord_trk, const tRtnMsg* p_rtn_msg);

	void onTickPx(uint32_t instr_hash, double tick_px);

	bool save(string day, bool force=false) {return acc_base_.save(day, force);}
	bool load() {return acc_base_.load();}

	vector<ModInstr> getAllInstr() {return acc_base_.getAllInstr();}
	vector<ModPrd> getAllPrd() {return acc_base_.getAllPrd();}
	
	ModInstr* getModInstr(uint32_t instr_hash) {return acc_base_.getModInstr(instr_hash);}

protected:
	bool isExistRiskPrd(uint32_t prd_hash);
	bool isExistRiskInstr(uint32_t instr_hash);
	bool regRiskPrd(const char* prd_name);
	bool regRiskInstr(const char* instr_name);

	char stg_name_[64] = {0};
	json j_conf_;
	unordered_map<uint32_t, RiskInstrTop> map_risk_instr_;
	unordered_map<uint32_t, RiskPrd> map_risk_prd_;
	RiskAcc risk_acc_;
	AccBase acc_base_;
};

#endif
