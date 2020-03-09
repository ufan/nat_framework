#include "CAvgSpread.h"
#include "PublicFun.h"
#include "CTimer.h"

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
    
    LOG_LVL(DEBUGLEVEL1, "avgspread: %f", avgspread_);
}
