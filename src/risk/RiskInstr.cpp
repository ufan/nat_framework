#include "RiskInstr.h"
#include "ATStructure.h"
#include "CTradeBaseInfo.h"
#include "ConfUtil.h"
#include "ATStructure.h"
#include "Logger.h"
#include <set>
#include <vector>
using namespace std;

bool RiskInstr::init(const json& j_conf, const tInstrumentInfo* p_instr_info, AccBase* ab)
{
	if (ab == nullptr)
	{
		ALERT("acc_base is nullptr !");
		return false;
	}
	p_unit_vol = &ab->map_instr_[p_instr_info->instr_hash].unit_vol;
	
	string instr_str = string(p_instr_info->instr);
	string prd_str = string(p_instr_info->product);
	
	try
	{
		// allowed_instrument
		const json& j = j_conf["/RiskForGeneral/allowed_instrument"_json_pointer];
		vector<string> vec_prd;
		for (auto& i : j) {
			vec_prd.push_back(i);
		}
		set<string> set_instr = CTradeBaseInfo::productOrInstrumentToInstrSet(vec_prd);
		if (set_instr.find(instr_str) != set_instr.end()) {
			is_allowed = true;
		}

		// allowed_order_size
		if (! ConfUtil<int>::getValue(&allowed_order_size_threshold, j_conf["/RiskForOrder/allowed_order_size"_json_pointer], prd_str, instr_str)) {
			return false;
		}

		// intensity_cycle_order_count
		if (! ConfUtil<int>::getValue(&intensity_cycle_order_count_threshold, j_conf["/RiskForOrder/intensity_cycle_order_count"_json_pointer], prd_str, instr_str)) {
			return false;
		}

		// intensity_cycle_time_span
		if (! ConfUtil<double>::getValue(&intensity_cycle_time_span_threshold, j_conf["/RiskForOrder/intensity_cycle_time_span"_json_pointer], prd_str, instr_str)) {
			return false;
		}
		intensity_cycle_time_span_threshold *= 1000000000.0;

		// allowed_long_volume
		if (! ConfUtil<int>::getValue(&long_volume_threshold, j_conf["/RiskForInstrument/allowed_long_volume"_json_pointer], prd_str, instr_str)) {
			return false;
		}

		// allowed_short_volume
		if (! ConfUtil<int>::getValue(&short_volume_threshold, j_conf["/RiskForInstrument/allowed_short_volume"_json_pointer], prd_str, instr_str)) {
			return false;
		}

		// allowed_net_volume
		if (! ConfUtil<int>::getValue(&net_volume_threshold, j_conf["/RiskForInstrument/allowed_net_volume"_json_pointer], prd_str, instr_str)) {
			return false;
		}
	}
	catch (...)
	{
		ALERT("can't read j_conf.");
		return false;
	}
	
	return true;
}

// 检查下单压力是否正常
inline bool RiskInstr::IsIntensityNormal(long exch_time) {
	if (intensity_cycle_order_count >= intensity_cycle_order_count_threshold
	 && exch_time - intensity_cycle_time_record < intensity_cycle_time_span_threshold) {
		return false;
	}
	return true;
}

// 检查订单大小是否正确
inline bool RiskInstr::IsSizeCorrect(int size) {
	return size <= allowed_order_size_threshold;
}

// 检查多头数量是否合理
inline bool RiskInstr::IsInstrumentLongVolumeCorrect(int vol) {
	return p_unit_vol->pos_long + p_unit_vol->pos_long_td_opn_ing + vol <= long_volume_threshold;
}

// 检查空头数量是否合理
inline bool RiskInstr::IsInstrumentShortVolumeCorrect(int vol) {
	return p_unit_vol->pos_short + p_unit_vol->pos_short_td_opn_ing + vol <= short_volume_threshold;
}

// 检查净数量是否合理
inline bool RiskInstr::IsInstrumentNetVolumeCorrect(int dir, int vol) {
	if (dir == AT_CHAR_Buy) {
		return p_unit_vol->pos_long - p_unit_vol->pos_short
				+ p_unit_vol->pos_short_yd_cls_ing + p_unit_vol->pos_long_td_opn_ing
				+ p_unit_vol->pos_short_td_cls_ing + vol
			<= net_volume_threshold;
	} else {
		return p_unit_vol->pos_short - p_unit_vol->pos_long
				+ p_unit_vol->pos_long_yd_cls_ing + p_unit_vol->pos_short_td_opn_ing
				+ p_unit_vol->pos_long_td_cls_ing + vol
			<= net_volume_threshold;
	}
}

int RiskInstr::check(int dir, int off, int vol, long nano) {
	if (! is_allowed ) {
		return -200;
	}
	if (! IsSizeCorrect(vol) ) {
		return -202;
	}
	if (! IsIntensityNormal(nano) ) {
		return -203;
	}
	if (off == AT_CHAR_Open) {
		if (dir == AT_CHAR_Buy) {
			if (! IsInstrumentLongVolumeCorrect(vol)) {
				return -210;
			}
		} else {
			if (! IsInstrumentShortVolumeCorrect(vol)) {
				return -211;
			}
		}
	}
	if (! IsInstrumentNetVolumeCorrect(dir, vol)) {
		return -212;
	}

	return 0;
}

int RiskInstr::parseAutoOffset(int dir, int& off, int vol) {
	switch (dir) {
		case AT_CHAR_Buy:
			if (p_unit_vol->pos_short_yd - p_unit_vol->pos_short_yd_cls_ing >= vol) {
				off = AT_CHAR_CloseYesterday;
			} else if (off != AT_CHAR_Close && long_volume_threshold - p_unit_vol->pos_long - p_unit_vol->pos_long_td_opn_ing >= vol) {
				off = AT_CHAR_Open;
			} else if (p_unit_vol->pos_short_td - p_unit_vol->pos_short_td_cls_ing >= vol) {
				off = AT_CHAR_CloseToday;
			} else {
				return -270;
			}
			break;
		case AT_CHAR_Sell:
			if (p_unit_vol->pos_long_yd - p_unit_vol->pos_long_yd_cls_ing >= vol) {
				off = AT_CHAR_CloseYesterday;
			} else if (off != AT_CHAR_Close && short_volume_threshold - p_unit_vol->pos_short - p_unit_vol->pos_short_td_opn_ing >= vol) {
				off = AT_CHAR_Open;
			} else if (p_unit_vol->pos_long_td - p_unit_vol->pos_long_td_cls_ing >= vol) {
				off = AT_CHAR_CloseToday;
			} else {
				return -271;
			}
			break;
		default:
			return -272;
	}
	return 0;
}

void RiskInstr::onNew(long nano)
{
	if (++intensity_cycle_order_count > intensity_cycle_order_count_threshold) {
		intensity_cycle_order_count = 1;
		intensity_cycle_time_record = nano;
	}
}
