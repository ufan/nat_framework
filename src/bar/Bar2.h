#ifndef SRC_BAR_BAR2_H_
#define SRC_BAR_BAR2_H_

#include "ATStructure.h"
#include <vector>

using namespace std;

class Bar2 : public Bar {
public:
	int isInCurBar(long base_sec_interval);
	bool isNeedUpdateOpen(const UnitedMarketData* p_umd);
	bool isNeedUpdatePre();
	bool isNeedUpdateOhlc(const UnitedMarketData* p_umd);
	bool isNeedSettle(const UnitedMarketData* p_umd);
	
	void updateOpen(const UnitedMarketData* p_umd);
	void updateBar(const UnitedMarketData* p_umd);
	void updatePre(const UnitedMarketData* p_umd);
	void settle(const UnitedMarketData* p_umd);
	
	long adjust_bob = 0;
	long adjust_eob = 0;
	long base_sec = 0;
	long pre_base_sec = 0;
	double pre_close = 0.0;
	int pre_cum_vol = 0;
	double pre_cum_turnover = 0.0;
	double pre_open_int = 0.0;
	long settle_sec = 0;
};

#endif