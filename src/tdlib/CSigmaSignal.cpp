#include "CSigmaSignal.h"
#include "PublicFun.h"
#include "math.h"
#include<CTimer.h>

//extern int log_id;
CSigmaSignal::CSigmaSignal()
{
    sigma_halflife_ = 5;
    bidsparsity_ = 0;
    asksparsity_ = 0;
    historyvol_ = 0;
    historyvolr2_ = 0;
    minlogVol_ = 0;
    spread_ = 0;
    std::cout << "CSigmaSignal::Init" <<endl;
}

CSigmaSignal::~CSigmaSignal(){}

void CSigmaSignal::GetSignal(const CMarketStruct& ms){
    spread_ = ms.askp_[0] - ms.bidp_[0];
    minlogVol_ = log(ms.askv_[0] > ms.bidv_[0] ? ms.bidv_[0]: ms.askv_[0]);
    // if first tick, then it should be the 
    if(ms.RetIs2ndTick_()){
        historyvolr2_ = pow((ms.askp_[0] + ms.bidp_[0] - ms.lastaskp_[0] - ms.lastbidp_[0])/2.0, 2);
    }
    else{
        historyvolr2_ = EWM(historyvolr2_, pow((ms.askp_[0] + ms.bidp_[0] - ms.lastaskp_[0] - ms.lastbidp_[0])/2.0, 2), sigma_halflife_, ms.RetWhichTick_());
    }
    
    historyvol_ = sqrt(historyvolr2_);
    if(LEVELCOUNT_ == 5){
        bidsparsity_ = ms.bidp_[0] - ms.bidp_[4];
        asksparsity_ = ms.askp_[4] - ms.askp_[0];
    }    
}

void CSigmaSignal::OnTick(const CMarketStruct& ms){
    GetSignal(ms);
//    string log = std::to_string((ms.askp_[0] + ms.bidp_[0])/2.0) + ',' + std::to_string((ms.lastaskp_[0] + ms.lastbidp_[0])/2.0)+ 
//                    ',' + std::to_string(historyvolr2_) + ',' + std::to_string(historyvol_) + ',' + std::to_string(ms.RetWhichTick_()) + ',' + parseNano(ms.exchtime_ * 1000000000, "%H:%M:%S");
//    fastLog(log_id, log);
}

double CSigmaSignal::GetSigma(double spread_coeff, double minlogvol_coeff, double historyvol_coeff, double bidsparse_coeff, double asksparse_coeff) const
{
    if(LEVELCOUNT_ == 1){
        return spread_* spread_coeff + minlogVol_ * minlogvol_coeff + historyvol_ * historyvol_coeff;
    }
    else{
        return spread_* spread_coeff + minlogVol_ * minlogvol_coeff + historyvol_ * historyvol_coeff +  bidsparsity_  * bidsparse_coeff + asksparsity_ * asksparse_coeff;
    }
}