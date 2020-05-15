#include "ThostFtdcTraderApi.h"
#include "CEncodeConv.h"
#include "unistd.h"
#include <csignal>
#include <iostream>
#include <string>
#include <cstring>

class PwdSpi : public CThostFtdcTraderSpi
{
 public:
  PwdSpi(CThostFtdcTraderApi* api) :
      fApiHandle(api),
      fRequestId(0),
      fOldPwd(""),
      fNewPwd(""),
      fBrokerID(""),
      fUserID("")
  {}

  ~PwdSpi() {}

	virtual void OnFrontConnected()
  {
    std::cout << "Front connected successfully!" << std::endl;

    // get passwords
    std::cout << "Enter the old password:" << std::endl;
    std::getline(std::cin, fOldPwd);
    std::cout << "Enter the new password:" << std::endl;
    std::getline(std::cin, fNewPwd);

    // sent request
    CThostFtdcUserPasswordUpdateField pwdUpdate;
    std::memset(&pwdUpdate, 0, sizeof(pwdUpdate));
    std::strcpy(pwdUpdate.BrokerID, fBrokerID.c_str());
    std::strcpy(pwdUpdate.UserID, fUserID.c_str());
    std::strcpy(pwdUpdate.OldPassword, fOldPwd.c_str());
    std::strcpy(pwdUpdate.NewPassword, fNewPwd.c_str());

    int ret = fApiHandle->ReqUserPasswordUpdate(&pwdUpdate, fRequestId++);
    if (ret)
      std::cout << "Can't send password changing request\n";
    else
      std::cout << "Waiting for password changing...\n";
  }

	virtual void OnFrontDisconnected(int nReason)
  {
    std::cout << "Disconnected ocurr!\n";
    Stop();
  }

	virtual void OnRspUserPasswordUpdate(CThostFtdcUserPasswordUpdateField *pUserPasswordUpdate, CThostFtdcRspInfoField *pRspInfo, int nRequestID, bool bIsLast)
  {
    if (pRspInfo != nullptr && pRspInfo->ErrorID != 0)
    {
      std::cout << "Password can't be changed : " << CEncodeConv::gbk2utf8(pRspInfo->ErrorMsg).c_str() << std::endl;
    }
    else{
      std::cout << "Password changed successfully" << std::endl;
      std::cout << "\tBroker ID: " << pUserPasswordUpdate->BrokerID << std::endl;
      std::cout << "\tUser ID: " << pUserPasswordUpdate->UserID << std::endl;
      std::cout << "\tOld Password: " << pUserPasswordUpdate->OldPassword << std::endl;
      std::cout << "\tNew Password: " << pUserPasswordUpdate->NewPassword << std::endl;
    }

    //
    Stop();
  }

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
  std::string fOldPwd;
  std::string fNewPwd;
  std::string fBrokerID;
  std::string fUserID;
};


///////////////////////////////////////////////

PwdSpi *pUserSpi = nullptr;

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
  std::cout << "-b broker_id -u user_id -f front_uri" << std::endl;

	exit(-1);
}

int main(int argc, char *argv[])
{
  // TThostFtdcBrokerIDType broker_id = "9999";
  std::string front_uri = "tcp://180.168.146.187:10130"; // default: Simnow 7/24 environment
  std::string broker_id = "9999"; // default: Simnow

  std::string user_id = "163973"; // default: my own account in Simnow
  std::string old_passwd = "";
  std::string new_passwd = "";

  int opt = 0;
  while ( (opt = getopt(argc,argv,"b:u:f:")) != -1 )
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
      case '?': {
        Usage(argv[0]);
      }
      default:
        break;
    }
  }

  if ( user_id.empty() || front_uri.empty() )
    Usage(argv[0]);

  // setup CTP
  auto* pUserApi = CThostFtdcTraderApi::CreateFtdcTraderApi();
  std::cout << "CTP API : " << pUserApi->GetApiVersion() << std::endl;

  pUserSpi = new PwdSpi(pUserApi);
  pUserSpi->fBrokerID = broker_id;
  pUserSpi->fUserID = user_id;
  pUserSpi->fOldPwd = old_passwd;
  pUserSpi->fNewPwd = new_passwd;

  pUserApi->RegisterSpi(pUserSpi);
  pUserApi->RegisterFront(const_cast<char*>(front_uri.c_str()));
  pUserApi->SubscribePublicTopic(THOST_TERT_QUICK); // need check
  pUserApi->SubscribePrivateTopic(THOST_TERT_QUICK); // need check

  pUserApi->Init();
  pUserApi->Join();

  return 0;
}
