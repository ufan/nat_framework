#ifndef SRC_RISK_RISKPRD_H
#define SRC_RISK_RISKPRD_H

#include "AccBase.h"
#include "ATStructure.h"
#include "json.hpp"
using json = nlohmann::json;

class RiskPrd
{
public:
	bool init(const json& j, const char* prd_name, AccBase* acc_base_);
	int check(int dir, int off, int vol);
	
	UnitVol* p_unit_vol = nullptr;

protected:
	bool IsProductLongVolumeCorrect(int vol);
	bool IsProductShortVolumeCorrect(int vol);
	bool IsProductNetVolumeCorrect(int di, int vol);

	int long_volume_threshold = 0;
	int short_volume_threshold = 0;
	int net_volume_threshold = 0;
};

#endif