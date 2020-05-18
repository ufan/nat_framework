#include "ThostFtdcMdApi.h"
#include "CEncodeConv.h"
#include "unistd.h"
#include <csignal>
#include <iostream>
#include <string>
#include <cstring>

class TestMdSpi : public CThostFtdcMdSpi
{
 public:
  TestMdSpi(CThostFtdcMdApi* api) :
      fApiHandle(api),
      fRequestId(0),
      fBrokerID(""),
      fUserID("")
  {}

  ~TestMdSpi() {}

  // First callback invoked by event loop thread after init()
	virtual void OnFrontConnected()
  {
    std::cout << "Front connected successfully!" << std::endl;

    // send login request
    CThostFtdcReqUserLoginField reqLogin;
    std::memset(&reqLogin, 0, sizeof(reqLogin));
    std::strcpy(reqLogin.TradingDay, "20200516"); // not necessary
    std::strcpy(reqLogin.UserID, fUserID.c_str());
    std::strcpy(reqLogin.BrokerID, fBrokerID.c_str());
    std::strcpy(reqLogin.Password, fPassword.c_str());
    std::strcpy(reqLogin.UserProductInfo, "pro_info"); // not necessary
    if(fApiHandle->ReqUserLogin(&reqLogin,fRequestId++)) {
      std::cout << "Failed sending user login request...\n";
    }
    else {
      std::cout << "Waiting for user login...\n";
      std::cout << "Password used: " << fPassword << std::endl;
    }
  }

	virtual void OnFrontDisconnected(int nReason)
  {
    std::cout << "Disconnected occured! reason = " << nReason << std::endl;
    Stop();
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
      std::cout << "\t Trading Day: " << pRspUserLogin->TradingDay << std::endl;
      std::cout << "\t Login Time: " << pRspUserLogin->LoginTime << std::endl;
      std::cout << "\t User ID: " << pRspUserLogin->UserID << std::endl;
      std::cout << "\t System Name: " << pRspUserLogin->SystemName << std::endl;
      std::cout << "\t Front ID: " << pRspUserLogin->FrontID << std::endl;
      std::cout << "\t Session ID: " << pRspUserLogin->SessionID << std::endl;
      std::cout << "\t MaxOrderRef: " << pRspUserLogin->MaxOrderRef << std::endl;
      std::cout << "\t SHFE Time: " << pRspUserLogin->SHFETime << std::endl;
      std::cout << "\t DCE Time: " << pRspUserLogin->DCETime << std::endl;
      std::cout << "\t CZCE Time: " << pRspUserLogin->CZCETime << std::endl;
      std::cout << "\t FFEX Time: " << pRspUserLogin->FFEXTime << std::endl;
      std::cout << "\t INE Time: " << pRspUserLogin->INETime << std::endl;
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
    // fApiHandle->Release();
  }


 public:
  CThostFtdcMdApi* fApiHandle;
  int fRequestId;
  std::string fBrokerID;
  std::string fUserID;
  std::string fPassword;
};


///////////////////////////////////////////////

TestMdSpi *pUserSpi = nullptr;

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
  std::string front_uri = "tcp://180.168.146.187:10131"; // default: Simnow 7/24 environment
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
  auto* pUserApi = CThostFtdcMdApi::CreateFtdcMdApi();
  std::cout << "CTP API : " << pUserApi->GetApiVersion() << std::endl;

  pUserSpi = new TestMdSpi(pUserApi);
  pUserSpi->fBrokerID = broker_id;
  pUserSpi->fUserID = user_id;
  pUserSpi->fPassword = passwd;

  // 1. Register Spi instance
  pUserApi->RegisterSpi(pUserSpi);

  // 2. Register front address
  pUserApi->RegisterFront(const_cast<char*>(front_uri.c_str()));

  // 3. Init event loop thread and request connection to front
  // seteuid(geteuid());
  pUserApi->Init();

  // 4. Waiting for the end of event loop thread
  pUserApi->Join();

  return 0;
}
