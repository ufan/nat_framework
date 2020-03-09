#ifndef CABM_H_
#define CABM_H_

class CABM
{
private:
    double F1(double x);
    double G1(double x, double xi);
    double G2(double x, double xi);   
    double G3(double x, double xi);     
    double Buy_I_F(double mu_t, double sigma_t, double delta);
    double Buy_I_D(double mu_t,double sigma_t,double delta);
    double BuyExpectedSt(double mu_t,double sigma_t,double delta);
    double BuyExpectedSt2(double mu_t,double sigma_t,double delta, double halfspread);
    double Sell_I_F(double mu_t,double sigma_t,double delta);
    double Sell_I_D(double mu_t,double sigma_t,double delta);
    double SellExpectedSt(double mu_t,double  sigma_t,double  delta);
    double SellExpectedSt2(double mu_t, double sigma_t, double delta, double halfspread);
public:
    CABM();
    ~CABM();
    double GetSellCABMExecutedProb(double mu_t, double sigma_t, double delta);
    double GetBuyCABMExecutedProb(double mu_t, double sigma_t, double delta);
    double ExpectedSt(double mu_t, double sigma_t, double delta, double halfspread, int direction);
};
#endif