#include<cmath>
#include "CABM.h"
#include<PublicFun.h>
#include <iostream>

CABM::CABM(){
}

CABM::~CABM(){}

double CABM::F1(double x){
    return 0.5 * erfc(-x * 1/sqrt(2)) * sqrt(2*PI);
}

double CABM::G1(double x, double xi){
    return exp(pow(xi,2))*(xi * F1(x-xi) - exp(-0.5* (pow(x-xi, 2))));
}

double CABM::G2(double x, double xi){
    return 1/xi *(exp(xi * x) * F1(x) - exp(0.5* (pow(xi, 2)))* F1(x - xi));
}

double CABM::G3(double x,double xi){
    return 1/xi *(exp(xi * x) *x*F1(x) - G2(x,xi) - G1(x,xi));
}

double CABM::Buy_I_F(double mu_t, double sigma_t, double delta){
    //double eps = 0.001;
    double eps = 0.2; // should be as certain quantile
    if (std::abs(mu_t) > eps){
        double f1 = mu_t/sigma_t;
        double f2 = (mu_t-delta)/sigma_t;
        double I1 = pow(sigma_t, 2) * (G1(f1, 2*f1) - G1(f2,2*f1));
        double I2 = (pow(sigma_t, 2) - pow(mu_t, 2))*(G2(f1, 2*f1) - G2(f2,2*f1));
        double I3 = 2*mu_t*sigma_t*(G3(f1, 2*f1) - G3(f2,2*f1));
        return (I1 + I2 + I3)* exp(-2*pow(f1, 2));
    }
    else{
        return delta * F1(-delta/sigma_t) * sigma_t;
    }
}

double CABM::Buy_I_D(double mu_t,double sigma_t,double delta){
    double eps = 0.2;
    //double eps = 0.001;
    if (std::abs(mu_t) > eps){
        double f1 = mu_t/sigma_t;   
        double I2 = mu_t *(G2(f1, 2*f1) - G2((mu_t-delta)/sigma_t,2*f1));
        double I1 = sigma_t * (F1(-f1) - F1(-(mu_t + delta)/sigma_t));
        return I2 * exp(-2*pow(f1, 2)) + I1;
    }
    else
        return (F1(0) - F1(-delta/sigma_t)) * sigma_t;
}

double CABM::BuyExpectedSt(double mu_t,double sigma_t,double delta){// here delta is the positive number which indicated -delta level 
    if (delta > 0){
        return Buy_I_F(mu_t, sigma_t, delta)/Buy_I_D(mu_t, sigma_t, delta);
    }
    else{
        double z0 = mu_t/sigma_t;
        return (pow(sigma_t,2) *z0 * exp(-0.5*pow(z0,2)) + (pow(sigma_t,2) + 2*mu_t *sigma_t *z0 - pow(mu_t,2)) *F1(z0))/(sigma_t *exp(-0.5* pow(z0,2)) + mu_t*F1(z0));
    }
}

double CABM::BuyExpectedSt2(double mu_t,double sigma_t,double delta, double halfspread){
    return halfspread + BuyExpectedSt(mu_t, sigma_t, delta + halfspread);
}

double CABM::Sell_I_F(double mu_t, double sigma_t,double delta){
    double eps = 0.2;
    //double eps = 0.001;
    if(std::abs(mu_t) > eps){
        double f1 = mu_t/sigma_t;
        double f2 = (mu_t+delta)/sigma_t;
        double I1 = -pow(sigma_t,2) * (G1(-f1, -2*f1) - G1(-f2, -2*f1));
        double I2 = (-pow(sigma_t,2) + pow(mu_t,2))*(G2(-f1, -2*f1) - G2(-f2,-2*f1));
        double I3 = 2*mu_t*sigma_t*(G3(-f1, -2*f1) - G3(-f2, -2*f1));
        return (I1 + I2 + I3)* exp(-2*pow(f1,2));
    }        
    else
        return -delta * F1(-delta/sigma_t) * sigma_t;
}

double CABM::Sell_I_D(double mu_t,double sigma_t,double delta){
    //double eps = 0.001;
    double eps = 0.2;
    if (std::abs(mu_t) > eps){
        double f1 = mu_t/sigma_t;
        double I2 = -mu_t *(G2(-f1, -2*f1) - G2(-(mu_t + delta)/sigma_t,-2*f1));
        double I1 = sigma_t *(F1(f1) - F1((mu_t - delta)/sigma_t));
        return I2 * exp(-2*pow(f1, 2)) + I1;
    }    
    else
        return (F1(delta/sigma_t) - F1(0)) * sigma_t;
}    

double CABM::SellExpectedSt(double mu_t,double  sigma_t,double  delta){ // here delta is the positive number which indicated -delta level 
    if (delta > 0){
        return Sell_I_F(mu_t, sigma_t, delta)/Sell_I_D(mu_t, sigma_t, delta);
    }
    else{
        double z0 = -mu_t/sigma_t;
        return (-pow(sigma_t,2) *z0 *exp(-0.5* pow(z0,2)) + (-pow(sigma_t,2)+2*mu_t *sigma_t *z0 + pow(mu_t,2))*F1(z0))/(sigma_t *exp(-0.5*pow(z0,2))- mu_t*F1(z0));
    }
}

double CABM::SellExpectedSt2(double mu_t, double sigma_t, double delta, double halfspread){ // here delta is the positive number which indicated -delta level
    return halfspread * (-1) + SellExpectedSt(mu_t, sigma_t, delta + halfspread);
}

double CABM::GetBuyCABMExecutedProb(double mu_t, double sigma_t, double delta){
    if(delta < 0){
        return 1.0;
    }
    return 1.0 - Buy_I_D(mu_t, sigma_t, delta) * 2/(sqrt(2*PI) * sigma_t);
}

double CABM::GetSellCABMExecutedProb(double mu_t, double sigma_t, double delta){
    if(delta <0){
        return 1.0;    
    }
    return 1.0 - Sell_I_D(mu_t, sigma_t, delta) * 2/(sqrt(2*PI) * sigma_t);
}

double CABM::ExpectedSt(double mu_t, double sigma_t, double delta, double halfspread, int direction){
    if (direction > 0){
        return BuyExpectedSt2(mu_t, sigma_t, delta, halfspread);
    }
    else{
        return SellExpectedSt2(mu_t, sigma_t, delta, halfspread);
    }
}
