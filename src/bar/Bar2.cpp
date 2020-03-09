#include "Bar2.h"
#include "BarHelper.h"

int Bar2::isInCurBar(long base_sec_interval)
{
	if (adjust_bob <= base_sec_interval && base_sec_interval < adjust_eob)
	{
		return 0;
	}
	else if (adjust_eob <= base_sec_interval)
	{
		return 1;
	}
	else
	{
		return -1;
	}
}

bool Bar2::isNeedUpdateOpen(const UnitedMarketData* p_umd)
{
	if (is_auction)
	{
		long sec = BarHelper::getBaseIntervalFromNano(p_umd->exch_time);
		if (bob <= sec && sec < eob)
		{
			return true;
		}
	}
	else
	{
		if (open == 0.0)
		{
			return true;
		}
	}
	return false;
}

bool Bar2::isNeedUpdatePre()
{
	return pre_close == 0.0;
}

bool Bar2::isNeedUpdateOhlc(const UnitedMarketData* p_umd)
{
	long sec = BarHelper::getBaseIntervalFromNano(p_umd->exch_time);
	if (bob <= sec && sec < eob)
	{
		return true;
	}
	else if (settle_sec != 0 && sec == settle_sec)
	{
		return true;
	}
	
	return false;
}

bool Bar2::isNeedSettle(const UnitedMarketData* p_umd)
{
	long sec = BarHelper::getBaseIntervalFromNano(p_umd->exch_time);
	if (is_auction)
	{
		if (bob <= sec && sec < eob)
		{
			return true;
		}
	}
	else
	{
		if (settle_sec != 0 && sec == settle_sec)
		{
			return true;
		}
	}
	return false;
}

void Bar2::updateOpen(const UnitedMarketData* p_umd)
{
	open = p_umd->last_px;
}

void Bar2::updateBar(const UnitedMarketData* p_umd)
{
	if (p_umd->last_px > high)
	{
		high = p_umd->last_px;
	}
	if (p_umd->last_px < low || low == 0.0)
	{
		low = p_umd->last_px;
	}
	close = p_umd->last_px;
	cum_vol = p_umd->cum_vol;
	cum_turnover = p_umd->cum_turnover;
	open_int = p_umd->open_interest;
	
	if (base_sec == 0)
	{
		base_sec = BarHelper::getBaseSec(p_umd->exch_time);
//		printf("updateBar, %ld, base:%ld\n", p_umd->exch_time, base_sec);
	}
}

void Bar2::updatePre(const UnitedMarketData* p_umd)
{
	pre_base_sec = BarHelper::getBaseSec(p_umd->exch_time);
	pre_close = p_umd->last_px;
	pre_cum_vol = p_umd->cum_vol;
	pre_cum_turnover = p_umd->cum_turnover;
	pre_open_int = p_umd->open_interest;
//	printf("updatePre, %s, %ld, pre_base:%ld\n", p_umd->instr_str, p_umd->exch_time, pre_base_sec);
}

void Bar2::settle(const UnitedMarketData* p_umd)
{
	if (open == 0.0)
	{
		open = high = low = close = pre_close;
		cum_vol = pre_cum_vol;
		cum_turnover = pre_cum_turnover;
		open_int = pre_open_int;
		base_sec = BarHelper::getBaseSec(p_umd->exch_time);
//		printf("settle, open==0.0, base = pre_base:%ld\n", pre_base_sec);
	}
	delta_close = close - pre_close;
//	printf("settle, close=%.2f, pre_close=%.2f\n", close, pre_close);
	vol = cum_vol - pre_cum_vol;
	turnover = cum_turnover - pre_cum_turnover;
	delta_open_int = open_int - pre_open_int;
	
	bob = (bob + base_sec) * 1000000000L;
	eob = (eob + base_sec) * 1000000000L;
//	printf("settle, base:%ld\n", base_sec);
}