#include "CAvgSpread.h"
#include "PublicFun.h"
#include<CTimer.h>

//extern int log_id;
CAvgSpread::CAvgSpread(){
    horizon_ = 0;
}

CAvgSpread::CAvgSpread(const CMarketStruct & ms, double horizon){
    horizon_ = horizon;
    avgspread_ = ms.askp_[0] - ms.bidp_[0];
}

CAvgSpread::~CAvgSpread(){}

void CAvgSpread::UpdateAvgspread(const CMarketStruct & ms){
    avgspread_ = EWM(avgspread_, ms.askp_[0] - ms.bidp_[0], horizon_, ms.RetWhichTick_()); 
}

double CAvgSpread::GetAvgspread(){
    //std::cout << "avgspread: " << avgspread_ << endl;
    return avgspread_;
}

void CAvgSpread::OnTick(const CMarketStruct & ms){
    UpdateAvgspread(ms);
    //std::cout << "OnTick avgspread: " << avgspread_ << ";" << ms.askp_[0] - ms.bidp_[0] << endl;
    //string log = std::to_string(avgspread_) + ',' + std::to_string(ms.askp_[0] - ms.bidp_[0]) + ',' + std::to_string(ms.RetWhichTick_()) + ',' + parseNano(ms.exchtime_ * 1000000000, "%H:%M:%S");
    //fastLog(log_id, log);
}