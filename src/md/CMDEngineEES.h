/*
 * CMDEngineEES.h
 *
 *  Created on: May 22, 2018
 *      Author: hongxu
 */

#ifndef SRC_MD_CMDENGINEEES_H_
#define SRC_MD_CMDENGINEEES_H_

#include "IMDEngine.h"
#include "EESQuoteApi.h"

class CMDEngineEES: public IMDEngine, public EESQuoteEvent
{
public:
	CMDEngineEES();
	virtual ~CMDEngineEES();

	bool connectTcp();

	bool initMulticast();

protected:
	virtual bool init();

	virtual bool start();

	virtual void join();

	virtual void release();

	virtual void subscribe(const vector<string> &instr);

	virtual void unsubscribe(const vector<string> &instr);
	virtual void OnEqsConnected();
	virtual void OnEqsDisconnected();
	virtual void OnLoginResponse(bool bsuccess, const char* pReason);
	virtual void OnSymbolRegisterResponse(EesEqsIntrumentType chInstrumentType, const char *pSymbol, bool bSuccess);
	virtual void OnSymbolUnregisterResponse(EesEqsIntrumentType chInstrumentType, const char* pSymbol, bool bSuccess);
	virtual void OnQuoteUpdated(EesEqsIntrumentType chInstrumentType, EESMarketDepthQuoteData* pDepthQuoteData);

protected:
	EESQuoteApi *api_ 		= nullptr;
	bool		is_login_	= false;
};

#endif /* SRC_MD_CMDENGINEEES_H_ */
