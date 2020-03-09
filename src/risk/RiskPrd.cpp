#include "RiskPrd.h"
#include "ConfUtil.h"
#include "Logger.h"
#include "MurmurHash2.h"
#include "ATStructure.h"
#include <string>
using namespace std;

bool RiskPrd::init(const json& j_conf, const char* prd_name, AccBase* ab)
{
	if (ab == nullptr)
	{
		ALERT("acc_base is nullptr !");
		return false;
	}
	
	uint32_t prd_hash = INSTR_NAME_TO_HASH(prd_name);	
	p_unit_vol = &ab->map_prd_[prd_hash].unit_vol;
	
	try
	{
		// allowed_long_volume
		if (! ConfUtil<int>::getValue(&long_volume_threshold, j_conf["/RiskForProduct/allowed_long_volume"_json_pointer], prd_name, "")) {
			return false;
		}

		// allowed_short_volume
		if (! ConfUtil<int>::getValue(&short_volume_threshold, j_conf["/RiskForProduct/allowed_short_volume"_json_pointer], prd_name, "")) {
			return false;
		}

		// allowed_net_volume
		if (! ConfUtil<int>::getValue(&net_volume_threshold, j_conf["/RiskForProduct/allowed_net_volume"_json_pointer], prd_name, "")) {
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

// 检查多头数量是否合理
inline bool RiskPrd::IsProductLongVolumeCorrect(int vol) {
	return p_unit_vol->pos_long + p_unit_vol->pos_long_td_opn_ing + vol <= long_volume_threshold;
}

// 检查空头数量是否合理
inline bool RiskPrd::IsProductShortVolumeCorrect(int vol) {
	return p_unit_vol->pos_short + p_unit_vol->pos_short_td_opn_ing + vol <= short_volume_threshold;
}

// 检查净数量是否合理
inline bool RiskPrd::IsProductNetVolumeCorrect(int dir, int vol) {
	if (dir == AT_CHAR_Buy) {
		return p_unit_vol->pos_long - p_unit_vol->pos_short
				+ p_unit_vol->pos_short_yd_cls_ing + p_unit_vol->pos_long_td_opn_ing + p_unit_vol->pos_short_td_cls_ing + vol
			<= net_volume_threshold;
	} else {
		return p_unit_vol->pos_short - p_unit_vol->pos_long
				+ p_unit_vol->pos_long_yd_cls_ing + p_unit_vol->pos_short_td_opn_ing + p_unit_vol->pos_long_td_cls_ing + vol
			<= net_volume_threshold;
	}
}

int RiskPrd::check(int dir, int off, int vol) {
	if (off == AT_CHAR_Open) {
		if (dir == AT_CHAR_Buy) {
			if (! IsProductLongVolumeCorrect(vol)) {
				return -300;
			}
		} else {
			if (! IsProductShortVolumeCorrect(vol)) {
				return -301;
			}
		}
	}
	if (! IsProductNetVolumeCorrect(dir, vol)) {
		return -302;
	}
	return 0;
}
