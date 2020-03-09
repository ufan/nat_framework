#ifndef CDRIFT_H_
#define CDRIFT_H_

#include<CBaseSignal.h>

class CDriftModel
{        
private:
    double start_mut_;
    double start_horizon_;
    double start_tradecoeff_[TOTAL_HORIZON_];
    double start_complogskew_coeff_;
    double start_bookcoeff_[TOTAL_HORIZON_];
    double start_OBDcoeff_[TOTAL_HORIZON_];
    
    double end_mut_;
    double end_horizon_;
    double end_tradecoeff_[TOTAL_HORIZON_];
    double end_complogskew_coeff_;
    double end_bookcoeff_[TOTAL_HORIZON_];
    double end_OBDcoeff_[TOTAL_HORIZON_];
    
    double current_mu_t_;
    
private:
    void GetStartEndMut(const CBaseSignal & basesignal);    

public:
    CDriftModel();
    ~CDriftModel();
    bool init(string config_path);
    void GetDrift(double timeleft, const CBaseSignal& basesignal);
    //double GetAdaptedStartDrift(const double logskew5,const double* trade,const double *book,const double *OBD) const;
    //double GetAdaptedEndDrift(const double logskew5,const double* trade,const double *book,const double *OBD) const;
    
public:
    void OnTick(const CBaseSignal& basesignal);
    double RetEndMut() const;
    double RetStartMut() const;
    double RetCurrentMut() const;
    double RetStartHorizon() const;
    double RetEndHorizon() const;
    double RetOneTickMut() const;
};
#endif
