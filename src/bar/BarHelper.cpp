#include "BarHelper.h"

long BarHelper::CZCE_nano = 0;
long BarHelper::DCE_nano = 0;
long BarHelper::SHFE_nano = 0;
long BarHelper::CFFEX_nano = 0;
long BarHelper::OTHER_nano = 0;
long BarHelper::intra_day_base_sec = 18 * 3600L;

void BarHelper::setIntraDayBaseSec(long sec)
{
	intra_day_base_sec = sec;
}

long BarHelper::getBaseSec(long nano) {
	return (nano / 1000000000L + 8*3600 - intra_day_base_sec) / 86400 * 86400 - 8*3600 + intra_day_base_sec;
}

long BarHelper::getBaseIntervalFromBjtSec(long bjt_sec) {
	return (bjt_sec - intra_day_base_sec + 86400) % 86400;
}

long BarHelper::getBaseIntervalFromNano(long nano) {
	long base_sec = getBaseSec(nano);
	return nano / 1000000000L - base_sec;
}
