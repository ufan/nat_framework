#ifndef CSIGMASIGNAL_H_
#define CSIGMASIGNAL_H_

#include "CMarketStruct.h"
class CSigmaSignal
{
public:
    double sigma_halflife_;
    int spread_;
    double minlogVol_;
    double historyvolr2_;
    double historyvol_;
    double bidsparsity_;
    double asksparsity_;
    void GetSignal(const CMarketStruct &ms);
    
public:
    CSigmaSignal();
    ~CSigmaSignal();    
    double GetSigma (double spread_coeff, double minlogvol_coeff, double historyvol_coeff, double bidsparse_coeff, double asksparse_coeff) const;
    void OnTick(const CMarketStruct &ms);
};
#endif