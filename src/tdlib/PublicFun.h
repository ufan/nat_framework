#ifndef CPUBLICFUN_H_
#define CPUBLICFUN_H_

#include "math.h"  

#define TOTAL_HORIZON_  5
#define HORIZON_USE_  3
#define TOTAL_DELTA_  4
#define LEVELCOUNT_ 1
#define PI 3.1415926
#define DURATION_LIMIT 0.05
#define MAX_OUTSTANDING_ORDERS 3
#define CANCEL_STATUS_EXIT -1
#define CANCEL_STATUS_NORMAL 0
#define TRADEVOL_HORIZON  60
#define LOSSVALUE_CHANGE_THRESHOLD 0.025
#define LOSSVALUE_DIFF_THRESHOLD 0.01

// 50 ms , once the duration < 50 ms then catch the current liquidity

// horizon_use_ is the first number of horizons used to produce signals
// total delta is number of deltas to test the minimizer of loss

inline double EWM(double signal, double newsignal, double halflife, int seq){ //seq is the order of the feed
    if(halflife >0){
        double weight = pow(2.0, -1.0/halflife);
        //signal = signal + (newsignal - signal) * (1 - weight)/(1- (seq > 20 ? 0 :pow(weight, seq)));
        signal = signal + (newsignal - signal) * (1 - weight)/(1-(seq > 10 * halflife ? 0 : pow(weight, seq)));
    }
    else{
        signal = newsignal;
    }    
    return signal;
}

inline double EWM_Unadjust(double signal, double newsignal, double halflife){
    double weight = pow(2.0, -1.0/halflife);
    signal = signal * weight + (1 - weight) * newsignal;
    return signal;
}

inline double LinearInterpolate(double startX, double startY, double endX, double endY, double X){
    double factor = (X - startX + 0.0)/(endX - startX);
    factor = factor > 1? 1: factor < 0 ? 0: factor;
    return factor * endY + (1.0 - factor) * startY;
}

inline int sgn(double x){
    return (x > 0) - (x < 0);
}


inline int GetExecutablePrice(int midpriceT2,int level,int direction){
    if(midpriceT2 %2 ==0){
        return midpriceT2/2 + level * direction;
    }
    else{
        return int(round(midpriceT2/2.0 + level * direction + (-0.01) * sgn(direction *(level + 0.01))));
    }
}

inline double LogLinearInterpolate(double startX, double startY, double endX, double endY, double X){

    if(startY <= 0 or endY <= 0){        
        return -1.0;
    }
        
    double log_startY = log(startY);
    double log_endY  = log(endY);
    double logY = LinearInterpolate(startX, log_startY, endX, log_endY, X);
    return exp(logY);
}


#endif /*CPUBLICFUN_H_ */