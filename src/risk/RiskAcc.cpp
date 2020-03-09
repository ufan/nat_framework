#include "RiskAcc.h"
#include "ATConstants.h"
#include "Logger.h"

bool RiskAcc::init(const json& j_conf, AccBase* ab)
{
	if (ab == nullptr)
	{
		ALERT("acc_base is nullptr !");
		return false;
	}
	
	p_mod_acc = &ab->mod_acc_;
	p_unit_amt = &ab->mod_acc_.unit_amt;
	
	try
	{
		unfilled_ord_cnt_threshold = j_conf["/RiskForGeneral/allowed_max_unfilled_order_number"_json_pointer];
		amt_long_threshold = j_conf["/RiskForAccount/allowed_long_amount"_json_pointer];
		amt_short_threshold = j_conf["/RiskForAccount/allowed_short_amount"_json_pointer];
		net_amt_threshold = j_conf["/RiskForAccount/allowed_net_amount"_json_pointer];
	}
	catch (...)
	{
		ALERT("can't read j_conf.");
		return false;
	}
	
	return true;
}

int RiskAcc::check(int dir, int off, double px, int vol, int vol_multiple) {
	if (! IsUnfilledOrderCountCorrect()) {
		return -100;
	}
	double amount = px * vol * vol_multiple;
	if (off == AT_CHAR_Open) {
		if (dir == AT_CHAR_Buy) {
			if (! IsLongAmountCorrect(amount)) {
				return -110;
			}
		} else {
			if (! IsShortAmountCorrect(amount)) {
				return -111;
			}
		}
	}
	if (! IsNetAmountCorrect(dir, amount)) {
		return -112;
	}
	return 0;
}

// 检查未完成订单数量是否未超过上限
inline bool RiskAcc::IsUnfilledOrderCountCorrect() {
	return p_mod_acc->unfilled_order_cnt < unfilled_ord_cnt_threshold;
}

// 检查多头金额是否正确
inline bool RiskAcc::IsLongAmountCorrect(double amount) {
	return p_unit_amt->amt_long + p_unit_amt->amt_long_td_add_ing + amount <= amt_long_threshold;
}

// 检查空头金额是否正确
inline bool RiskAcc::IsShortAmountCorrect(double amount) {
	return p_unit_amt->amt_short + p_unit_amt->amt_short_td_add_ing + amount <= amt_short_threshold;
}

// 检查轧差金额是否正确
inline bool RiskAcc::IsNetAmountCorrect(int dir, double amount) {
	if (dir == AT_CHAR_Buy) {
		return p_unit_amt->amt_long - p_unit_amt->amt_short
				+ p_unit_amt->amt_long_td_add_ing + p_unit_amt->amt_short_yd_sub_ing + p_unit_amt->amt_short_td_sub_ing
				+ amount
			<= net_amt_threshold;
	} else {
		return p_unit_amt->amt_short - p_unit_amt->amt_long
				+ p_unit_amt->amt_short_td_add_ing + p_unit_amt->amt_short_yd_sub_ing + p_unit_amt->amt_short_td_sub_ing
				+ amount
			<= net_amt_threshold;
	}
}
