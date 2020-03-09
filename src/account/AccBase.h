#ifndef SRC_ACCOUNT_ACCBASE_H
#define SRC_ACCOUNT_ACCBASE_H

#include "ModInstr.h"
#include "ModPrd.h"
#include "ModAcc.h"
#include "ATStructure.h"
#include <unordered_map>
#include <string>
#include <vector>
using namespace std;

class AccBase
{
public:
	bool init(const char* name, const json& j);
	bool regInstr(const char* instr_name);

	void onSwitchDay();
	bool save(string day, bool force=false);
	bool load();

	void onOrderTrack(const tOrderTrack* p_ord_trk);
	void onNew(int dir, int off, double px, int vol, uint32_t instr_hash);
	void onRtn(const tOrderTrack* p_ord_trk, const tRtnMsg* p_rtn_msg);

	void onTickPx(uint32_t instr_hash, double tick_px);

	ModAcc* getModAcc();
	ModInstr* getModInstr(uint32_t instr_hash);
	ModPrd* getModPrd(uint32_t prd_hash);
	vector<ModInstr> getAllInstr();
	vector<ModPrd> getAllPrd();
	
	UnitAmt* getAccUnitAmt();
	UnitPnl* getAccUnitPnl();
	UnitVol* getInstrUnitVol(uint32_t instr_hash);
	UnitAmt* getInstrUnitAmt(uint32_t instr_hash);
	UnitPx* getInstrUnitPx(uint32_t instr_hash);
	UnitPnl* getInstrUnitPnl(uint32_t instr_hash);
	UnitVol* getPrdUnitVol(uint32_t prd_hash);
	UnitAmt* getPrdUnitAmt(uint32_t prd_hash);
	UnitPnl* getPrdUnitPnl(uint32_t prd_hash);
	
	unordered_map<uint32_t, ModInstr> map_instr_;
	unordered_map<uint32_t, ModPrd> map_prd_;
	ModAcc mod_acc_;

protected:
	bool isExistModPrd(uint32_t prd_hash);
	bool isExistModInstr(uint32_t instr_hash);
	bool regModPrd(const char* prd_name);
	bool regModInstr(const char* instr_name);
	bool getJsonFilePath(int t, string day, string &path);
	json to_json();
	bool from_json(json& j);
	
	json j_conf_;
	char acc_name_[64] = {0};
};

#endif