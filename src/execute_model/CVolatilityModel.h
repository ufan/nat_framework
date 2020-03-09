#ifndef CVOLATILITY_H_
#define CVOLATILITY_H_

#include<CBaseSignal.h>
#include<CSigmaSignal.h>

class CVolatilityModel
{    
public:
    CVolatilityModel();
    ~CVolatilityModel();
    
private:
    double fullsigma_;
    double instant_sigma_;
    double full_historyvol_coeff_;
    double full_spread_coeff_;
    double full_asksparisty_coeff_;
    double full_bidsparsity_coeff_;
    double full_minlogvol_coeff_;
    double full_intercept_;
    double full_sigmabase_;
    
private:
    double onetick_sigma_;
    double onetick_historyvol_coeff_;
    double onetick_spread_coeff_;
    double onetick_asksparisty_coeff_;
    double onetick_bidsparsity_coeff_;
    double onetick_minlogvol_coeff_;
    double onetick_intercept_;
    double onetick_sigma_base_;

private:
    void Evaluate_full_volatility(const CSigmaSignal& sigmasignal);
    void Evaluate_volatility_onetick(const CSigmaSignal& sigmasignal);
    
public:
    bool init(string config);
    double RetInstantVolatility(double timeleft, double duration) const;
    void   OnTick(const CSigmaSignal& sigmasignal);
    double RetOnetickVolatility() const;
};
#endif
