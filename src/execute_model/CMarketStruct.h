#ifndef CMARKETSTRUCT_H_
#define CMARKETSTRUCT_H_

#include "PublicFun.h"

class CMarketStruct
{

private: 
    bool IsLastTickReady(const UnitedMarketData *md);
    bool is_last_tick_ready_;
    
public:
    int  whichtick_;
    int askv_[5];
    int askp_[5];
    int bidv_[5];
    int bidp_[5];
    int lastaskp_[5];
    int lastaskv_[5];
    int lastbidp_[5];
    int lastbidv_[5];
    
    int tradevol_;
    int turnover_;    
    double last_cumturnover_;
    int last_cumvol_;
    double avgprice_;
    double exchtime_;
    double lastexchtime_; // exchtime in seconds
    
    double 	ticksize_;
    int 	multiplier_;
    int		exchange_;


public:
    CMarketStruct();
    ~CMarketStruct();
    bool init(const char *instr);
    void OnTick(const UnitedMarketData *md);
    void UpdateLastTickInfo(const UnitedMarketData *md);
    bool RetIsLastReady_() const;
    bool RetIs2ndTick_()const;
    int RetWhichTick_()const;
    int GetTradePriceRelatedVolume(int orderprice, int direction);
};

#endif
