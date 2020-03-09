#ifndef SRC_RISK_RISKINSTRSTG_H
#define SRC_RISK_RISKINSTRSTG_H

#include "RiskInstr.h"

class RiskInstrStg : public RiskInstr
{
public:
	bool init(const json& j_conf, const tInstrumentInfo* p_instr_info, AccBase* ab);
	int check(int dir, int off, int vol, double px, double ask, double bid, long nano);

protected:
	bool IsPriceCorrect(double px, double ask, double bid);
	
	double allowed_price_margin_threshold = 0.0;
};

#endif