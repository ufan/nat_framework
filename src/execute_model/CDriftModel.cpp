#include "CDriftModel.h"
#include "json.hpp"
#include <fstream>
#include "CTimer.h"
#include "Logger.h"

using json = nlohmann::json;


CDriftModel::CDriftModel(){
    // set the initialzied parameters
    start_horizon_ = 3;
    end_horizon_ = 1;
    start_complogskew_coeff_ = 0;
    end_complogskew_coeff_ = 0;
    for(int count=0; count< TOTAL_HORIZON_; count++){
        start_tradecoeff_[count] = 0;
        start_bookcoeff_[count] = 0;
        start_OBDcoeff_[count]  = 0;
        end_tradecoeff_[count]  = 0;
        end_bookcoeff_[count]   = 0;
        end_OBDcoeff_[count]    = 0;        
    }    
    current_mu_t_ = 0;
}

CDriftModel::~CDriftModel()
{}

bool CDriftModel::init(string config_path)
{
    std::ifstream f1(config_path);
    if (!f1)
    {
    	LOG_ERR("open %s failed.", config_path.c_str());
    	return false;
    }
    string content((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    f1.close();

    json param_reader = json::parse(content);
    start_complogskew_coeff_ = param_reader["startmu_coeff"][0];
    start_tradecoeff_[0] = param_reader["startmu_coeff"][1];
    start_tradecoeff_[1] = param_reader["startmu_coeff"][2];
    start_tradecoeff_[2] = param_reader["startmu_coeff"][3];
    start_bookcoeff_[0] = param_reader["startmu_coeff"][4];
    start_bookcoeff_[1] = param_reader["startmu_coeff"][5];
    start_bookcoeff_[2] = param_reader["startmu_coeff"][6];

    end_complogskew_coeff_ = param_reader["endmu_coeff"][0];
    end_tradecoeff_[0] = param_reader["endmu_coeff"][1];
    end_tradecoeff_[1] = param_reader["endmu_coeff"][2];
    end_tradecoeff_[2] = param_reader["endmu_coeff"][3];
    end_bookcoeff_[0] = param_reader["endmu_coeff"][4];
    end_bookcoeff_[1] = param_reader["endmu_coeff"][5];
    end_bookcoeff_[2] = param_reader["endmu_coeff"][6];

	return true;
}

void CDriftModel::GetStartEndMut(const CBaseSignal & bs){
    // set the initialzied parameters
    start_mut_ = bs.GetMut(start_complogskew_coeff_, start_tradecoeff_, start_bookcoeff_, start_OBDcoeff_);
    end_mut_ =   bs.GetMut(end_complogskew_coeff_,   end_tradecoeff_,   end_bookcoeff_,   end_OBDcoeff_);
    //string log = std::to_string(start_mut_) + ',' + std::to_string(end_mut_);    
    //fastLog(log_id, log);
}

void CDriftModel::GetDrift(double timeleft, const CBaseSignal & bs){
    current_mu_t_ = LinearInterpolate(start_horizon_, start_mut_, end_horizon_, end_mut_, timeleft);       
}

/*double CDriftModel::GetAdaptedStartDrift(const double logskew5,const double* trade,const double *book,const double *OBD) const{
    double adpated_drift = start_complogskew_coeff_ * logskew5;
    for(int count=0; count< HORIZON_USE_; count++){
        adpated_drift += start_tradecoeff_[count] * trade[count];
        adpated_drift += start_bookcoeff_[count] * book[count];
        if(LEVELCOUNT_ == 5)
            adpated_drift += start_OBDcoeff_[count] * OBD[count];        
    }        
    return adpated_drift;
}

double CDriftModel::GetAdaptedEndDrift(const double logskew5,const double* trade,const double *book,const double *OBD) const{
    double adpated_drift = end_complogskew_coeff_ * logskew5;
    for(int count=0; count< HORIZON_USE_; count++){
        adpated_drift += end_tradecoeff_[count] * trade[count];
        adpated_drift += end_bookcoeff_[count] * book[count];
        if(LEVELCOUNT_ == 5)
            adpated_drift += end_OBDcoeff_[count] * OBD[count];        
    }    
    return adpated_drift;
} */

void CDriftModel::OnTick(const CBaseSignal& bs){
    GetStartEndMut(bs);
    LOG_LVL(DEBUGLEVEL2, "start_mut: %f, end_mut: %f, start_horizon: %f, end_horizon: %f", start_mut_, end_mut_, start_horizon_, end_horizon_);
}

double CDriftModel::RetEndMut() const{
    return end_mut_;
}
double CDriftModel::RetStartMut() const{
    return start_mut_;
}

double CDriftModel::RetCurrentMut() const{
    return current_mu_t_;
}

double CDriftModel::RetStartHorizon() const{
    return start_horizon_;
}

double CDriftModel::RetEndHorizon() const{
    return end_horizon_;
}

double CDriftModel::RetOneTickMut() const{
    return end_mut_;
}
