#ifndef SRC_ACCOUNT_MODINSTR_H
#define SRC_ACCOUNT_MODINSTR_H

#include "UnitVol.h"
#include "UnitAmt.h"
#include "UnitPx.h"
#include "UnitPnl.h"
#include "ModPrd.h"
#include "ModAcc.h"
#include <string>
using namespace std;

class ModInstr
{
public:
	bool init(const char* name, int vm);
	
	json to_json();
	bool from_json(json& j);
	void onSwitchDay();

	void onTickPx(double tick_px);
	void onNew(int dir, int off, double px, int vol);
	void onCxl(int dir, int off, double px, int cxl_vol);
	void onTrd(int dir, int off, double px, int vol, int vol_traded, double trd_px, int trd_vol);
	string getInstrName() {return instr_name;}
	
	UnitVol unit_vol;
	UnitAmt unit_amt;
	UnitPx unit_px;
	UnitPnl unit_pnl;
	
	ModPrd* p_prd = nullptr;
	ModAcc* p_acc = nullptr;

	char instr_name[32] = {0};
	int vol_multiple = 0;
};

#endif