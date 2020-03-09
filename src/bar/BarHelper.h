#ifndef SRC_BAR_BARHELPER_H_
#define SRC_BAR_BARHELPER_H_

const long DELAY_SEC = 2L;

class BarHelper {
public:
	static void setIntraDayBaseSec(long sec);
	static long getBaseSec(long nano);
	static long getBaseIntervalFromBjtSec(long bjt_sec);
	static long getBaseIntervalFromNano(long nano);
	
	static long CZCE_nano;
	static long DCE_nano;
	static long SHFE_nano;
	static long CFFEX_nano;
	static long OTHER_nano;
	static long intra_day_base_sec;
};

#endif