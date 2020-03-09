#include "CLiquidityModel.h"
#include "Logger.h"
#include <fstream>

CLiquidityModel::CLiquidityModel(){
    for(int count=0; count< TOTAL_DELTA_; count++){
        buy_drift_coeff_[count] = 0;
        buy_sigma_coeff_[count] = 0;
        buy_intercept_[count] = 0;
        sell_drift_coeff_[count] = 0;
        sell_sigma_coeff_[count] = 0;
        sell_intercept_[count] = 0;
    }
}

CLiquidityModel::~CLiquidityModel()
{}

bool CLiquidityModel::init(string config)
{
    std::ifstream f1(config);
    if (!f1)
    {
    	LOG_ERR("open %s failed.", config.c_str());
    	return false;
    }
    string content((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    f1.close();
    json param_reader = json::parse(content);
    int DELTA[TOTAL_DELTA_];

    for(int count=0; count< TOTAL_DELTA_; count++){
        DELTA[count] = param_reader["Delta"][count];
    }

    for(int count=0; count< TOTAL_DELTA_; count++){
        buy_intercept_[count]    = param_reader["Buy"][std::to_string(DELTA[count])][0];
        buy_drift_coeff_[count]  = param_reader["Buy"][std::to_string(DELTA[count])][1];
        buy_sigma_coeff_[count]  = param_reader["Buy"][std::to_string(DELTA[count])][2];
        sell_intercept_[count]   = param_reader["Sell"][std::to_string(DELTA[count])][0];
        sell_drift_coeff_[count] = param_reader["Sell"][std::to_string(DELTA[count])][1];
        sell_sigma_coeff_[count] = param_reader["Sell"][std::to_string(DELTA[count])][2];
    }

    return true;
}

int CLiquidityModel::GetOrdervolume(int leastordervolume, int maxordervolume, int block, int deltaIndex, int direction, const CDriftModel& dm, const CVolatilityModel& sigma){
    sigma_onetick_ = sigma.RetOnetickVolatility();
    mut_onetick_ = dm.RetOneTickMut();
    int ordervolume = 0;
    if(direction >0){
        ordervolume = int((buy_drift_coeff_[deltaIndex] * mut_onetick_ + sigma_onetick_ * buy_sigma_coeff_[deltaIndex] + buy_intercept_[deltaIndex])/block) * block;
        ordervolume = ordervolume > maxordervolume ? maxordervolume : ordervolume < leastordervolume ? leastordervolume: ordervolume;
    }else{
        ordervolume = int((sell_drift_coeff_[deltaIndex] * mut_onetick_ + sigma_onetick_ * sell_sigma_coeff_[deltaIndex] + sell_intercept_[deltaIndex])/block) * block;
        ordervolume = ordervolume > maxordervolume ? maxordervolume : ordervolume < leastordervolume ? leastordervolume: ordervolume;
    }
    
    return ordervolume;
}
