#ifndef SRC_ACCOUNT_MODPRD_H
#define SRC_ACCOUNT_MODPRD_H

#include "UnitVol.h"
#include "UnitAmt.h"
#include "UnitPnl.h"
#include <string>
using namespace std;

class ModPrd
{
public:
	bool init(const char* name);

	json to_json();
	bool from_json(json& j);
	void onSwitchDay();
	string getPrdName() {return prd_name;}
	
	char prd_name[32] = {0};
	UnitVol unit_vol;
	UnitAmt unit_amt;
	UnitPnl unit_pnl;
};

#endif