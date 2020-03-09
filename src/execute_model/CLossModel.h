#ifndef CLOSSMODEL_H_
#define CLOSSMODEL_H_

#include "CMarketStruct.h"
#include "CBaseSignal.h"
#include "CSigmaSignal.h"
#include "CDriftModel.h"
#include "CLogitPModel.h"
#include "CVolatilityModel.h"
#include "CLiquidityModel.h"
#include "CABM.h"
#include "CAvgSpread.h"
#include "PublicFun.h"
#include "CStrategy.h"
#include <vector>

class CLossModel
{
public:
	struct tExecOrder
	{
	    std::unordered_map<int, int> orderpvmap_;
	    double loss_level_[TOTAL_DELTA_];
	    int 	placed_vol_ = 0;
	    double  total_duration_ = 0;
	    int     priceup_ = 10000;
	    int     pricedown_ = 1;
	    bool    isOrdering_ = false;
	    int     direction_ = 1;
	    int     totalsize_ = 0;
	    double 	duration_ = 0.0;
	    double 	timeleft_ = 0.0;
	    double 	order_placetime_ = 0;
	    int     bestordervol_ = 0;
	    int     bestorderprice_ = 0;
	    long 	QorderID_[MAX_OUTSTANDING_ORDERS];
	    int 	QorderID_size_ = 0;
	    int 	cancel_status_ = CANCEL_STATUS_NORMAL;
	    int		order_id_ = -1;

	    tExecOrder()
	    {
	        for(int count=0; count< TOTAL_DELTA_; count++){
	            loss_level_[count] = 0.0;
	        }

	        for(int count = 0; count < MAX_OUTSTANDING_ORDERS; ++count){
	            QorderID_[count] = -1;
	        }
	    }
	};

public:
    CLossModel(CStrategy *powner);
    ~CLossModel();
    bool init(string instr, string config);
    bool loadLossModelConfig(string config);
    
    int GetSizeplacedOnPrice(tExecOrder &eo, int orderprice);
    double Lossfun(tExecOrder &eo, int level_index, int orderprice);
    void PlaceOrder(tExecOrder &eo);
    int CancelOrderAPIStatus(tExecOrder &eo, double to_orderprice);
    void GetBestOrderPriceVolume(tExecOrder &eo);
    void ProcessExit();
    int GetLossMinimizer(tExecOrder &eo, int newplacedsize);
    void OnTick(const UnitedMarketData *md);
    void GetTimeleft();

    void sys_on_tick(const UnitedMarketData* md);
    void sys_on_time(long nano);
	bool sys_on_rtn(const tRtnMsg* rtn);
	void sys_on_switch_day(string day);

    int OnExecution(double priceup, double pricedown, int placedsize, int direction, double duration);
    void exitExecution(int execid);
    void cancelExecution(int execid);

    int sendOrder(tExecOrder &eo, double price, int vol, int dir, int off, int acc_idx=0)
    {
    	int ret = p_owner_->sendOrder(instr_hash_, price, vol, dir, off, acc_idx);
    	if(ret >= 0) eo.placed_vol_ += vol;
    	return ret;
    }

protected:
    CMarketStruct ms_;
    CBaseSignal bs_;
    CSigmaSignal ss_;
    CDriftModel dm_;
    CLogitPModel lp_;
    CVolatilityModel vm_;
    CLiquidityModel lq_;
    CABM  abm_;
    CAvgSpread avgsp_;

protected:
    vector<tExecOrder>	orders_;

    int extra_slippage_;
    int levels_[TOTAL_DELTA_];

    int leastordervolume_;
    int maxordervolume_;
    int volume_multiplier_;
    double ticksize_;
    double multiplier;
    uint32_t instr_hash_  = 0;
    double valid_duration_;

    CStrategy *p_owner_;
    long   lst_tick_nano_ = 0;
};
#endif
