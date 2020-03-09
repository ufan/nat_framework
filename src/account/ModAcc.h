#ifndef SRC_ACCOUNT_MODACC_H
#define SRC_ACCOUNT_MODACC_H

#include "UnitVol.h"
#include "UnitAmt.h"
#include "UnitPnl.h"

class ModAcc
{
public:
	json to_json();
	bool from_json(json& j);
	void onSwitchDay();

	int unfilled_order_cnt = 0;
	UnitAmt unit_amt;
	UnitPnl unit_pnl;
};

#endif