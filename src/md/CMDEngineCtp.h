/*
 * CMDEngineCtp.h
 *
 *  Created on: 2018年4月27日
 *      Author: hongxu
 */

#ifndef SRC_MD_CMDENGINECTP_H_
#define SRC_MD_CMDENGINECTP_H_

#include "IMDEngine.h"
#include "ThostFtdcMdApi.h"
#include "ThostFtdcUserApiStruct.h"

class CMDEngineCtp: public IMDEngine, public CThostFtdcMdSpi
{
public:
	CMDEngineCtp();
	virtual ~CMDEngineCtp();

	// ctp callbacks
	void OnFrontConnected();
	void OnFrontDisconnected(int nReason);
	void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRspSubMarketData(CThostFtdcSpecificInstrumentField *pSpecificInstrument, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast);
	void OnRtnDepthMarketData(CThostFtdcDepthMarketDataField *p_data);

protected:

	virtual bool init();

	virtual bool start();

	virtual void join();

	virtual void release();

	virtual void subscribe(const vector<string> &instr);

	virtual void unsubscribe(const vector<string> &instr);

private:
	CThostFtdcMdApi			*p_api_;
	int 					reqid_;
	bool					is_login_	= false;
};

#endif /* SRC_MD_CMDENGINECTP_H_ */

