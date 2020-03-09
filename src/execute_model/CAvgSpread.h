#ifndef CAVGSPREAD_H_
#define CAVGSPREAD_H_

#include"CMarketStruct.h"

class CAvgSpread{
// average spread model
private:
    double avgspread_;
    double horizon_;
    void   UpdateAvgspread(const CMarketStruct & ms);
    
public:
    CAvgSpread();
    CAvgSpread(const CMarketStruct & ms,double horizon);
    ~CAvgSpread();
    double GetAvgspread();
    void OnTick(const CMarketStruct & ms);
};
#endif