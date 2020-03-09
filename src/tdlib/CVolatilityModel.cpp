#include "CVolatilityModel.h"
#include <fstream>

//extern int log_id;
CVolatilityModel::CVolatilityModel(){
    // set the initialzied parameters
    onetick_historyvol_coeff_ = 0;
    onetick_spread_coeff_ = 0;
    onetick_asksparisty_coeff_ = 0;
    onetick_bidsparsity_coeff_ = 0;
    onetick_minlogvol_coeff_ = 0;
    onetick_intercept_ = 0.1;
    onetick_sigma_base_ = 0.1;
    
    //now on the coeff 
    full_historyvol_coeff_ = 0;
    full_spread_coeff_ = 0;
    full_asksparisty_coeff_ = 0;
    full_bidsparsity_coeff_ = 0;
    full_minlogvol_coeff_ = 0;
    full_intercept_ = 0.1;
    full_sigmabase_ = 0.1;
    
    std::ifstream f1("CVolatilityCoeff.cnf");
    if (!f1)
		cout << "can't read file: " << endl;
    string content((std::istreambuf_iterator<char>(f1)), std::istreambuf_iterator<char>());
    f1.close();
    json param_reader;
    param_reader = json::parse(content);
    //std::cout << param_reader << endl;
    onetick_intercept_ = param_reader["onetick_intercept"];
    onetick_spread_coeff_ = param_reader["onetick_spread_coeff"];
    onetick_minlogvol_coeff_ = param_reader["onetick_minlogvol_coeff"];
    onetick_historyvol_coeff_ = param_reader["onetick_historyvol_coeff"];
    onetick_sigma_base_ = param_reader["onetick_sigmaBase"];
    
    full_intercept_ = param_reader["full_intercept"];
    full_spread_coeff_ = param_reader["full_spread_coeff"];
    full_minlogvol_coeff_ = param_reader["full_minlogvol_coeff"];
    full_historyvol_coeff_ = param_reader["full_historyvol_coeff"];
    full_sigmabase_ = param_reader["full_sigmaBase"];    
    
    if(LEVELCOUNT_ == 5){
        onetick_bidsparsity_coeff_ = param_reader["onetick_bidsparisty_coeff"];
        onetick_asksparisty_coeff_ = param_reader["onetick_asksparisty_coeff"];
        full_bidsparsity_coeff_ = param_reader["full_bidsparisty_coeff"];
        full_asksparisty_coeff_ = param_reader["full_asksparisty_coeff"];            
    }
    std::cout << "CVolatilityModel::Init" <<endl;
}

CVolatilityModel::~CVolatilityModel(){
    // set the initialzied parameters    
}

void CVolatilityModel::Evaluate_full_volatility(const CSigmaSignal & sigmasignal){
    // set the initialzied parameters
    fullsigma_ = sigmasignal.GetSigma(full_spread_coeff_, full_minlogvol_coeff_, full_historyvol_coeff_, full_bidsparsity_coeff_, full_asksparisty_coeff_) + full_intercept_;
}

void CVolatilityModel::Evaluate_volatility_time(double timeleft, double duration, const CSigmaSignal& sigmasignal){    
    instant_sigma_ = fullsigma_ * timeleft/duration;
    if(instant_sigma_ < full_sigmabase_){
        instant_sigma_ = full_sigmabase_;
    }
}

void CVolatilityModel::Evaluate_volatility_onetick(const CSigmaSignal& sigmasignal){
    onetick_sigma_ = sigmasignal.GetSigma(onetick_spread_coeff_, onetick_minlogvol_coeff_, onetick_historyvol_coeff_, onetick_bidsparsity_coeff_, onetick_asksparisty_coeff_) + onetick_intercept_;
    if(onetick_sigma_ < onetick_sigma_base_){
        onetick_sigma_ = onetick_sigma_base_;
    }
}

void CVolatilityModel::OnTick(double timeleft, double total_time, const CSigmaSignal& sigmasignal){
    Evaluate_full_volatility(sigmasignal);
    Evaluate_volatility_time(timeleft, total_time, sigmasignal);
    Evaluate_volatility_onetick(sigmasignal);
}

double CVolatilityModel::RetInstantVolatility() const{
    return instant_sigma_;
}

double CVolatilityModel::RetOnetickVolatility() const{
    return onetick_sigma_;
}