#ifndef SRC_RISK_RISKACC_H
#define SRC_RISK_RISKACC_H

#include "AccBase.h"
#include "json.hpp"
using json = nlohmann::json;

class RiskAcc
{
public:
	bool init(const json& j, AccBase* ab);
	int check(int dir, int off, double px, int vol, int vol_multiple);

protected:
	bool IsUnfilledOrderCountCorrect();
	bool IsLongAmountCorrect(double amount);
	bool IsShortAmountCorrect(double amount);
	bool IsNetAmountCorrect(int dir, double amount);

	int unfilled_ord_cnt_threshold = 0;
	double amt_long_threshold = 0;
	double amt_short_threshold = 0;
	double net_amt_threshold = 0;
	
	ModAcc* p_mod_acc = nullptr;
	UnitAmt* p_unit_amt = nullptr;
};

#endif