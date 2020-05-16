#include "ThostFtdcTraderApi.h"
#include "CEncodeConv.h"
#include "unistd.h"
#include <csignal>
#include <iostream>
#include <string>
#include <cstring>

class TestTraderSpi : public CThostFtdcTraderSpi
{
 public:
  TestTraderSpi(CThostFtdcTraderApi* api) :
      fApiHandle(api),
      fRequestId(0),
      fBrokerID(""),
      fUserID("")
  {}

  ~TestTraderSpi() {}

  // First callback invoked by event loop thread after init()
	virtual void OnFrontConnected()
  {
    std::cout << "Front connected successfully!" << std::endl;

    // check authenticate method 
    CThostFtdcReqUserAuthMethodField reqAuthMethod;
    std::memset(&reqAuthMethod, 0, sizeof(reqAuthMethod));
    std::strcpy(reqAuthMethod.TradingDay, "20200516");
    std::strcpy(reqAuthMethod.BrokerID, fBrokerID.c_str());
    std::strcpy(reqAuthMethod.UserID, fUserID.c_str());
    if (fApiHandle->ReqUserAuthMethod(&reqAuthMethod, fRequestId++))
      std::cout << "Can't send AuthMethod request\n";
    else
      std::cout << "Waiting for authentication method...\n";

    // sent authenticate request
    CThostFtdcReqAuthenticateField reqAuth;
    std::memset(&reqAuth, 0, sizeof(reqAuth));
    std::strcpy(reqAuth.BrokerID, fBrokerID.c_str());
    std::strcpy(reqAuth.UserID, fUserID.c_str());
    std::strcpy(reqAuth.UserProductInfo, "pro_info");
    std::strcpy(reqAuth.AuthCode, "0000000000000000");
    std::strcpy(reqAuth.AppID, "simnow_client_test");

    if (fApiHandle->ReqAuthenticate(&reqAuth, fRequestId++))
      std::cout << "Can't send authenticate request\n";
    else
      std::cout << "Waiting for authentication...\n";
  }

	virtual void OnFrontDisconnected(int nReason)
  {
    std::cout << "Disconnected occured! reason = " << nReason << std::endl;
    Stop();
  }

	virtual void OnRspUserAuthMethod(CThostFtdcRspUserAuthMethodField *pRspUserAuthMethod, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
  {
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
      std::cout << "Request AuthMethod failed: " << CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str() << std::endl;
    }
    else{
      // print out authenticate response
      std::cout << "Requset AuthMethod succeed: " << pRspUserAuthMethod->UsableAuthMethod << std::endl;
    }
  }

  // Callback on authenticate request
	virtual void OnRspAuthenticate(CThostFtdcRspAuthenticateField *pRspAuthenticateField, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
  {
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
      std::cout << "Authenticate failed: " << CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str() << std::endl;
    }
    else{
      // print out authenticate response
      std::cout << "Authenticate succeed:\n";
      std::cout << "\tBroker ID = " << pRspAuthenticateField->BrokerID << std::endl;
      std::cout << "\tUser ID = " << pRspAuthenticateField->UserID << std::endl;
      std::cout << "\tUser Product Info = " << pRspAuthenticateField->UserProductInfo << std::endl;
      // std::cout << "\tAuth Info = " << pRspAuthenticateField->AuthInfo << std::endl;
      // std::cout << "\tIsResult = " << pRspAuthenticateField->IsResult << std::endl;
      std::cout << "\tApp ID = " << pRspAuthenticateField->AppID << std::endl;
      std::cout << "\tApp Type = " << pRspAuthenticateField->AppType << std::endl;
      // std::cout << "\tClientIPAddress = " << pRspAuthenticateField->ClientIPAddress << std::endl;

      // send login request
      CThostFtdcReqUserLoginField reqLogin;
      strcpy(reqLogin.TradingDay, "20200516");
      strcpy(reqLogin.UserID, fUserID.c_str());
      strcpy(reqLogin.BrokerID, fBrokerID.c_str());
      strcpy(reqLogin.Password, fPassword.c_str());
			strcpy(reqLogin.UserProductInfo, "pro_info");
      if(fApiHandle->ReqUserLogin(&reqLogin,fRequestId++)) {
        std::cout << "Failed sending user login request...\n";
      }
      else {
        std::cout << "Waiting for user login...\n";
        std::cout << "Password used: " << fPassword << std::endl;
      }
    }
  }

  // Callback on user login
	virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
  {
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
      std::cout << "Login failed: " << CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str() << std::endl;
    }
    else{
      std::cout << "Login successfully!\n";
    }
  }

  // Callback on error response
	virtual void OnRspError(CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
  {
    std::cout << "Error occured: " << CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str() << std::endl;
  }

	virtual void OnHeartBeatWarning(int nTimeLapse)
  {
    std::cout << "Heart Beat Warning: Long time no message\n";
  }

  void Stop()
  {
    CThostFtdcUserLogoutField req = {};
    std::strcpy(req.BrokerID, fBrokerID.c_str());
    std::strcpy(req.UserID, fUserID.c_str());

    if (fApiHandle->ReqUserLogout(&req, fRequestId++))
    {
      std::cout << "Failed request logout\n";
    }
    fApiHandle->Release();
  }

	// virtual void OnRspUserLogin(CThostFtdcRspUserLoginField *pRspUserLogin, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast) {};


 public:
  CThostFtdcTraderApi* fApiHandle;
  int fRequestId;
  std::string fBrokerID;
  std::string fUserID;
  std::string fPassword;
};


///////////////////////////////////////////////

TestTraderSpi *pUserSpi = nullptr;

void signal_handler(int signum)
{
	if(pUserSpi) pUserSpi->Stop();
}

void setup_signal_callback()
{
  std::signal(SIGTERM, signal_handler);
  std::signal(SIGINT, signal_handler);
  std::signal(SIGHUP, signal_handler);
  std::signal(SIGQUIT, signal_handler);
  std::signal(SIGKILL, signal_handler);
}

void Usage(const char *cmd)
{
  std::cout << "Usage: " << cmd << " ";
  std::cout << "-b broker_id -u user_id -f front_uri -p passwd" << std::endl;

	exit(-1);
}

int main(int argc, char *argv[])
{
  // TThostFtdcBrokerIDType broker_id = "9999";
  std::string front_uri = "tcp://180.168.146.187:10130"; // default: Simnow 7/24 environment
  std::string broker_id = "9999"; // default: Simnow

  std::string user_id = "163973"; // default: my own account in Simnow
  std::string passwd = "NowSim88418";

  int opt = 0;
  while ( (opt = getopt(argc,argv,"b:u:f:p:")) != -1 )
  {
    switch (opt) {
      case 'b': {
        broker_id = optarg;
        break;
      }
      case 'u': {
        user_id = optarg;
        break;
      }
      case 'f': {
        front_uri = optarg;
        break;
      }
      case 'p': {
        passwd = optarg;
        break;
      }
      case '?': {
        Usage(argv[0]);
      }
      default:
        break;
    }
  }

  if ( user_id.empty() || front_uri.empty() || passwd.empty())
    Usage(argv[0]);

  // setup CTP
  auto* pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
  std::cout << "CTP API : " << pUserApi->GetApiVersion() << std::endl;

  pUserSpi = new TestTraderSpi(pUserApi);
  pUserSpi->fBrokerID = broker_id;
  pUserSpi->fUserID = user_id;
  pUserSpi->fPassword = passwd;

  // 1. Register Spi instance
  pUserApi->RegisterSpi(pUserSpi);

  // 2. Register front address
  pUserApi->RegisterFront(const_cast<char*>(front_uri.c_str()));

  // 3. Subscribe to public and private topics
  pUserApi->SubscribePublicTopic(THOST_TERT_QUICK); // need check
  pUserApi->SubscribePrivateTopic(THOST_TERT_QUICK); // need check

  // 4. Init event loop thread and request connection to front
  pUserApi->Init();

  // 5. Waiting for the end of event loop thread
  pUserApi->Join();

  return 0;
}
