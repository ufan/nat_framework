#ifndef CLOGITMODEL_H_
#define CLOGITMODEL_H_

#include "CBaseSignal.h"
class CLogitPModel
{
public:
    CLogitPModel();
    ~CLogitPModel();
    
private:
//public:
    double buy_startlogit_prob_;
    double buy_endlogit_prob_;
    double buy_startprob_intercept_[TOTAL_DELTA_];
    double buy_endprob_intercept_[TOTAL_DELTA_];
    double buy_startprob_logskewCoeff_[TOTAL_DELTA_];
    double buy_endprob_logskewCoeff_[TOTAL_DELTA_];
    double buy_startprob_tradecoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double buy_endprob_tradecoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double buy_startprob_bookcoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double buy_endprob_bookcoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double buy_startprob_OBDcoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double buy_endprob_OBDcoeff_[TOTAL_DELTA_][HORIZON_USE_];
    //double buy_startprob_sigmacoeff_[TOTAL_DELTA_];
    //double buy_endprob_sigmacoeff_[TOTAL_DELTA_];
    
    double sell_startlogit_prob_;
    double sell_endlogit_prob_;
    double sell_startprob_intercept_[TOTAL_DELTA_];
    double sell_endprob_intercept_[TOTAL_DELTA_];
    double sell_startprob_logskewCoeff_[TOTAL_DELTA_];
    double sell_endprob_logskewCoeff_[TOTAL_DELTA_];
    double sell_startprob_tradecoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double sell_endprob_tradecoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double sell_startprob_bookcoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double sell_endprob_bookcoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double sell_startprob_OBDcoeff_[TOTAL_DELTA_][HORIZON_USE_];
    double sell_endprob_OBDcoeff_[TOTAL_DELTA_][HORIZON_USE_];
    //double sell_startprob_sigmacoeff_[TOTAL_DELTA_];
    //double sell_endprob_sigmacoeff_[TOTAL_DELTA_];
    
    int startsize_;
    int endsize_;    
    int IsProbSet1_[TOTAL_DELTA_];
    
private:
    double logitProb(double kernel);
    
public:
    double EvaluateProb(int level_index, int tradesize, const CBaseSignal& bs, double timeleft, double duration, int direction);
    bool init(string config);
    
};
#endif
