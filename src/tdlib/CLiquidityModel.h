#ifndef C_LIQUIDITY_H_
#define C_LIQUIDITY_H_

#include<PublicFun.h>
#include<CDriftModel.h>
#include<CVolatilityModel.h>

class CLiquidityModel
{
private:
    double buy_drift_coeff_[TOTAL_DELTA_];// use getDriftMethod 
    double buy_sigma_coeff_[TOTAL_DELTA_]; // use getsigmaMethod 
    double buy_intercept_[TOTAL_DELTA_];
    
    double sell_drift_coeff_[TOTAL_DELTA_];// use getDriftMethod 
    double sell_sigma_coeff_[TOTAL_DELTA_]; // use getsigmaMethod 
    double sell_intercept_[TOTAL_DELTA_];    

    double mut_onetick_;
    double sigma_onetick_;

public:
    CLiquidityModel(int *DELTA);
    CLiquidityModel();
    ~CLiquidityModel();
    int GetOrdervolume(int leastordervolume, int maxordervolume, int multiplier, int deltaIndex, int direction, const CDriftModel & dm, const CVolatilityModel & sigma);    
};
#endif