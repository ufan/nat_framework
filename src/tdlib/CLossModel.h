#ifndef CLOSSMODEL_H_
#define CLOSSMODEL_H_

#include<CMarketStruct.h>
#include<CBaseSignal.h>
#include<CSigmaSignal.h>
#include<CDriftModel.h>
#include<CLogitPModel.h>
#include<CVolatilityModel.h>
#include<CLiquidityModel.h>
#include<CABM.h>
#include<CAvgSpread.h>
#include<PublicFun.h>

class CLossModel: public CStrategy
{
private:
    CMarketStruct ms_;
    CBaseSignal bs_;
    CSigmaSignal ss_;
    CDriftModel dm_;
    CLogitPModel lp_;
    CVolatilityModel vm_;
    CLiquidityModel lq_;
    CABM  abm_;
    CAvgSpread avgsp_;
    
private:
    std::unordered_map<int, int> orderpvmap_;
    int extra_slippage_;
    int levels_[TOTAL_DELTA_];
    double loss_level_[TOTAL_DELTA_];
    int leastordervolume_;
    int maxordervolume_;
    int volume_multiplier_;
    double ticksize_;
    double multiplier;
    uint32_t instr_hash_;

private:
    double valid_duration_;
    double total_duration_;
    int    priceup_;
    int    pricedown_;
    bool   isOrdering_;
    int    direction_;
    int    totalsize_;
    
private:
    double duration_;
    double timeleft_;    
    double order_placetime_;
    int    bestordervol_;
    int    bestorderprice_;
    UnitVol* uv_;    
    
private:
    long QorderID_[MAX_OUTSTANDING_ORDERS];
    int QorderID_size_;
    //int lastorderprice_;
    int cancel_status_;
    
private:
    int GetSizeplacedOnPrice(int direction, int orderprice);    
    double Lossfun(int level_index, int direction, double timeleft, int orderprice);
    void PlaceOrder();
    int CancelOrderAPIStatus(double to_orderprice);
    void GetBestOrderPriceVolume();
    void ProcessExit();
    int GetLossMinimizer(int direction, double timeleft, int newplacedsize);
    void OnTick(const UnitedMarketData *md);
    void GetTimeleft();
public:
    CLossModel();
    ~CLossModel();
    virtual void sys_on_tick(const UnitedMarketData* md);
	virtual void sys_on_time(long nano);
	virtual void sys_on_rtn(const tRtnMsg* rtn);
	virtual void sys_on_switch_day(string day);
    void OnExecution(int priceup, int pricedown, int placedsize, int direction, double duration); 
};
#endif
