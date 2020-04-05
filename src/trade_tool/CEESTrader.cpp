#include <dlfcn.h>
#include "CGlobalParameter.h"
#include "CEESTrader.h"
#include "CConfig.h"
#include "SimpleLogger.h"
#include "EnumStatus.h"
#include "CEncodeConv.h"

static const char* parseSide(EES_SideType side_type)
{
    const char *side = NULL;
    switch(side_type)
    {
    case EES_SideType_open_long: side = "买单开今"; break;
    case EES_SideType_close_today_long: side = "卖单平今"; break;
    case EES_SideType_close_today_short: side = "买单平今"; break;
    case EES_SideType_open_short: side = "卖单开今"; break;
    case EES_SideType_close_ovn_short: side = "买单平昨"; break;
    case EES_SideType_close_ovn_long: side = "卖单平昨"; break;
    case EES_SideType_force_close_ovn_short: side = "买单强平昨"; break;
    case EES_SideType_force_close_ovn_long: side = "卖单强平昨"; break;
    case EES_SideType_force_close_today_short: side = "买单强平今"; break;
    case EES_SideType_force_close_today_long: side = "卖单强平今"; break;
    case EES_SideType_opt_exec: side = "期权行权"; break;
    default: side = "Unknown";
    }
    return side;
}

static string parseOrderStatus(EES_OrderState state)
{
    string status;
    if(EES_OrderStatus_shengli_accept & state) status += "EES已接受;";
    if(EES_OrderStatus_mkt_accept & state) status += "市场已接受;";
    if(EES_OrderStatus_executed & state) status += "已成交或部分成交;";
    if(EES_OrderStatus_cancelled & state) status += "已撤销;";
    else if(EES_OrderStatus_cxl_requested & state) status += "撤销中;";
    if(EES_OrderStatus_closed & state) status += "已关闭;";
    return status;
}

CEESTrader::CEESTrader() : status_(INIT), trade_api_(NULL), handle_(NULL), distory_fun_(NULL), req_id_(0)
{

}

CEESTrader::~CEESTrader()
{
    Close();
}

bool CEESTrader::init()
{
    status_ = INIT;
    if(!loadEESLib()) return false;
    if(!connect()) return false;
    if(!getAccount()) return false;
    if(accounts_.empty())
    {
        LOG_ERR("No account found under this logon_id");
        return false;
    }
    if(!getSymbolList()) return false;

    CGlobalParameter::terminal_lock_ = false;
    return true;
}

bool CEESTrader::loadEESLib()
{
    CConfig *p_cnf = CGlobalParameter::getConfig();
    string lib_path = p_cnf->getVal<string>("EES", "lib_path");
    if(lib_path[lib_path.size() - 1] != '/') lib_path.push_back('/');
    lib_path += EES_TRADER_DLL_NAME;
	handle_ =  dlopen(lib_path.c_str(), RTLD_LAZY);
	if (!handle_)
	{
		LOG_ERR("load library(%s) failed.\n", EES_TRADER_DLL_NAME);
		return false;
	}

	funcCreateEESTraderApi createFun = (funcCreateEESTraderApi)dlsym(handle_, CREATE_EES_TRADER_API_NAME);
	if (!createFun)
	{
		LOG_ERR("get function addresss(%s) failed!\n", CREATE_EES_TRADER_API_NAME);
		return false;
	}

	distory_fun_ = (funcDestroyEESTraderApi)dlsym(handle_, DESTROY_EES_TRADER_API_NAME);
	if (!distory_fun_)
	{
		LOG_ERR("get function addresss(%s) failed!\n", DESTROY_EES_TRADER_API_NAME);
		return false;
	}

	trade_api_ = createFun();
	if (!trade_api_)
	{
		LOG_ERR("create trade API object failed!\n");
		return false;
	}

	return true;
}

bool CEESTrader::connect()
{
    CConfig *p_cnf = CGlobalParameter::getConfig();
    string trade_ip = p_cnf->getVal<string>("EES", "trade_ip");
    int trade_port = p_cnf->getVal<int>("EES", "trade_port");
    string query_ip = p_cnf->getVal<string>("EES", "query_ip");
    int query_port = p_cnf->getVal<int>("EES", "query_port");
    int trade_udp_port = p_cnf->getVal<int>("EES", "trade_udp_port");
    string local_ip = p_cnf->getVal<string>("EES", "local_ip");
    int local_port = p_cnf->getVal<int>("EES", "local_udp_port");

    EES_TradeSvrInfo svninfo;
    strcpy(svninfo.m_remoteTradeIp,  trade_ip.c_str());
    svninfo.m_remoteTradeTCPPort = trade_port;
    strcpy(svninfo.m_remoteQueryIp,  query_ip.c_str());
    svninfo.m_remoteQueryTCPPort = query_port;
    svninfo.m_remoteTradeUDPPort = trade_udp_port;
    strcpy(svninfo.m_LocalTradeIp,  local_ip.c_str());
    svninfo.m_LocalTradeUDPPort = local_port;

	RESULT ret_err = trade_api_->ConnServer(svninfo, (EESTraderEvent*)this);
	if (ret_err != NO_ERROR)
	{
		LOG_ERR("connect to REM server failed!\n");
		return false;
	}

	int waitTime = 0;//等待超时
	while (status_ != LOGIN && status_ != STATUS_ERR)
	{
		usleep(100000);
		waitTime++;
		if (waitTime >= 50)//5秒超时
		{
			LOG_ERR("wait for logon response timeout!\n");
			return false;
		}
	}

	return (LOGIN == status_);
}

void CEESTrader::OnConnection(ERR_NO errNo, const char* pErrStr)
{
	if (errNo != NO_ERROR)
	{
        string msg = CEncodeConv::gbk2utf8(pErrStr);
		LOG_ERR("connect to rem server failed(%d), %s!\n", errNo, msg.c_str());
		return;
	}
	status_ = CONNECT;
	login();
}

void CEESTrader::login()
{
	if (!trade_api_)
	{
		LOG_ERR("INVALID api object\n");
		return;
	}

    CConfig *p_cnf = CGlobalParameter::getConfig();
    string logon_id = p_cnf->getVal<string>("EES", "logon_id");
    string pwd = p_cnf->getVal<string>("EES", "passwd");
    string mac = p_cnf->getVal<string>("EES", "local_mac");
    string name = p_cnf->getVal<string>("EES", "name");
	trade_api_->UserLogon(logon_id.c_str(), pwd.c_str(), name.c_str(), mac.c_str());

    user_id_ = strtol(logon_id.c_str(), NULL, 0);
}

bool CEESTrader::getSymbolList()
{
    status_ = ON_QUERY;
	RESULT ret_err = trade_api_->QuerySymbolList();
	if (ret_err != NO_ERROR)
	{
        status_ = WAIT_QUERY;
		LOG_ERR("QuerySymbolList failed!\n");
		return false;
	}
    while(ON_QUERY == status_) usleep(50000);
    return true;
}

void CEESTrader::UnloadLib()
{
	if (trade_api_)
	{
		distory_fun_(trade_api_);
		trade_api_ = NULL;
		distory_fun_ = NULL;
	}

	if (handle_)
	{
		dlclose(handle_);
		handle_ = NULL;
	}
}

void CEESTrader::Close()
{
	if (trade_api_)
	{
		trade_api_->DisConnServer();
	}

	UnloadLib();
}

void CEESTrader::OnDisConnection(ERR_NO errNo, const char* pErrStr)
{
    string msg = CEncodeConv::gbk2utf8(pErrStr);
	LOG_ERR("disconnect from rem server(%d), %s!\n", errNo, msg.c_str());
	status_ = STATUS_ERR;
}

void CEESTrader::OnUserLogon(EES_LogonResponse* pLogon)
{

	if (pLogon->m_Result != NO_ERROR)
	{
		status_ = STATUS_ERR;
		LOG_ERR("logon failed, result=%d\n", pLogon->m_Result);
		return;
	}
	status_ = LOGIN;
	LOG_TRACE("logon successfully, trading date(%u), max token(%d)\n", pLogon->m_TradingDate, pLogon->m_MaxToken);
}

int CEESTrader::sendOrder(string instrument, double price, int volume, int direction, int offset)
{
    EES_EnterOrderField order;
    memcpy(order.m_Account, accounts_[0].m_Account, sizeof(EES_Account));
    if(direction == OD_Buy)
    {
        switch(offset)
        {
        case OO_Open: order.m_Side = EES_SideType_open_long; break;
        case OO_CloseToday: order.m_Side = EES_SideType_close_today_short; break;
        case OO_Close: order.m_Side = EES_SideType_close_ovn_short; break;
        default: ;
        }
    }
    else
    {
        switch(offset)
        {
        case OO_Open: order.m_Side = EES_SideType_open_short; break;
        case OO_CloseToday: order.m_Side = EES_SideType_close_today_long; break;
        case OO_Close: order.m_Side = EES_SideType_close_ovn_long; break;
        default: ;
        }
    }
    strncpy(order.m_Symbol, instrument.c_str(), sizeof(order.m_Symbol));
    order.m_Symbol[sizeof(order.m_Symbol) - 1] = '\0';
    if(symbol_info_.find(order.m_Symbol) == symbol_info_.end())
    {
        LOG_ERR("%s not in today symbol list.", order.m_Symbol);
        return -1;
    }
    order.m_Exchange = symbol_info_[order.m_Symbol];
    order.m_Price = price;
    order.m_Qty = volume;

    trade_api_->GetMaxToken(&(order.m_ClientOrderToken));
    order.m_ClientOrderToken++;

    RESULT ret_err = trade_api_->EnterOrder(&order);
    if (ret_err != NO_ERROR)
    {
        LOG_ERR("EnterOrder failed!: %d\n", ret_err);
        return -1;
    }

    stringstream ss;
    ss << order.m_Symbol << "|" << parseSide(order.m_Side) << "|" << price << "|" << volume;
    id_inst_map_[make_pair(user_id_, order.m_ClientOrderToken)] = ss.str();
    return 0;
}

int CEESTrader::deleteOrder(string ref)
{
    EES_CancelOrder order;

    stringstream ss(ref);
    string account;
    ss >> account >> order.m_MarketOrderToken;
    strncpy(order.m_Account, account.c_str(), sizeof(order.m_Account));
    order.m_Account[sizeof(order.m_Account) - 1] = '\0';
    order.m_Quantity = 0;

    RESULT ret_err = trade_api_->CancelOrder(&order);
    if (ret_err != NO_ERROR)
    {
        LOG_ERR("CancelOrder failed!: %d\n", ret_err);
        return -1;
    }
    return 0;
}

bool CEESTrader::getAccount()
{
    accounts_.clear();
    status_ = ON_QUERY;
	RESULT ret_err = trade_api_->QueryUserAccount();
	if (ret_err != NO_ERROR)
	{
        status_ = WAIT_QUERY;
		LOG_ERR("QueryUserAccount failed!: %d\n", ret_err);
		return false;
	}
    while(ON_QUERY == status_) usleep(50000);
    return true;
}

int CEESTrader::qryAccount()
{
    if(!getAccount())
    {
        CGlobalParameter::terminal_lock_ = false;
        return -1;
    }

    printf("账户       初始权益     总可用资金   占用保证金   冻结保证金   已扣手续费   冻结总手续费 昨仓保证金   总平仓盈亏   总持仓盈亏\n");
    for(uint32_t i = 0; i < accounts_.size(); ++i)
    {
        EES_AccountInfo & acc = accounts_[i];
        status_ = ON_QUERY;
        RESULT ret_err = trade_api_->QueryAccountBP(acc.m_Account, req_id_++);
        if (ret_err != NO_ERROR)
        {
            status_ = WAIT_QUERY;
            LOG_ERR("QueryAccountBP %s failed!: %d\n", acc.m_Account, ret_err);
            continue;
        }
        while(ON_QUERY == status_) usleep(50000);
    }
    CGlobalParameter::terminal_lock_ = false;
    return 0;
}

int CEESTrader::qryPosition(const char *inst_idstr)
{
    printf("账户       合约               方向 隔夜仓数量 冻结昨仓量 今仓量     冻结今仓量 隔夜占用金   今仓占用金\n");
    for(uint32_t i = 0; i < accounts_.size(); ++i)
    {
        EES_AccountInfo & acc = accounts_[i];
        status_ = ON_QUERY;
        RESULT ret_err = trade_api_->QueryAccountPosition(acc.m_Account, req_id_++);
        if (ret_err != NO_ERROR)
        {
            status_ = WAIT_QUERY;
            LOG_ERR("QueryAccountPosition %s failed!: %d\n", acc.m_Account, ret_err);
            continue;
        }
        while(ON_QUERY == status_) usleep(50000);
    }
    CGlobalParameter::terminal_lock_ = false;
    return 0;
}

int CEESTrader::qryOrder()
{
    printf("ID    UserID     ClientToken MarketToken 方向       数量       类别     合约               价格         成交量     状态\n");
    id_ = 0;
	CGlobalParameter::order_id_map_.clear();
    id_inst_map_.clear();
    for(uint32_t i = 0; i < accounts_.size(); ++i)
    {
        EES_AccountInfo & acc = accounts_[i];
        status_ = ON_QUERY;
        RESULT ret_err = trade_api_->QueryAccountOrder(acc.m_Account);
        if (ret_err != NO_ERROR)
        {
            status_ = WAIT_QUERY;
            LOG_ERR("QueryAccountOrder %s failed!: %d\n", acc.m_Account, ret_err);
            continue;
        }
        while(ON_QUERY == status_) usleep(50000);
    }
    CGlobalParameter::terminal_lock_ = false;
    return 0;
}

int CEESTrader::qryTrade()
{
    printf("UserID     ClientToken MarketToken 成交量     价格         合约\n");
    for(uint32_t i = 0; i < accounts_.size(); ++i)
    {
        EES_AccountInfo & acc = accounts_[i];
        status_ = ON_QUERY;
        RESULT ret_err = trade_api_->QueryAccountOrderExecution(acc.m_Account);
        if (ret_err != NO_ERROR)
        {
            status_ = WAIT_QUERY;
            LOG_ERR("QueryAccountTrade %s failed!: %d\n", acc.m_Account, ret_err);
            continue;
        }
        while(ON_QUERY == status_) usleep(50000);
    }
    CGlobalParameter::terminal_lock_ = false;
    return 0;
}

void CEESTrader::OnQueryUserAccount(EES_AccountInfo* pAccoutnInfo, bool bFinish)
{
    if(bFinish)
    {
        status_ = WAIT_QUERY;
    }
    else
    {
        accounts_.push_back(*pAccoutnInfo);
    }
}

void CEESTrader::OnQueryAccountBP(const char* pAccount, EES_AccountBP* pAccoutnPosition, int nReqId )
{
    printf("%-10s %-12.2f %-12.2f %-12.2f %-12.2f %-12.2f %-12.2f %-12.2f %-12.2f %-12.2f\n", pAccoutnPosition->m_account, pAccoutnPosition->m_InitialBp,
        pAccoutnPosition->m_AvailableBp, pAccoutnPosition->m_Margin, pAccoutnPosition->m_FrozenMargin, pAccoutnPosition->m_CommissionFee,
        pAccoutnPosition->m_FrozenCommission, pAccoutnPosition->m_OvnInitMargin, pAccoutnPosition->m_TotalLiquidPL, pAccoutnPosition->m_TotalMarketPL);
    status_ = WAIT_QUERY;
}

void CEESTrader::OnQueryAccountPosition(const char* pAccount, EES_AccountPosition* pAccoutnPosition, int nReqId, bool bFinish)
{
    if(bFinish)
    {
        status_ = WAIT_QUERY;
    }
    else
    {
        const char *dir = NULL;
        switch(pAccoutnPosition->m_PosiDirection)
        {
        case EES_PosiDirection_long: dir = "多头"; break;
        case EES_PosiDirection_short: dir = "空头"; break;
        default: dir = "Unknown";
        }
        printf("%-10s %-18s %-4s %-10u %-10u %-10u %-10u %-12.2f %-12.2f\n",
            pAccoutnPosition->m_actId, pAccoutnPosition->m_Symbol, dir, pAccoutnPosition->m_OvnQty,
            pAccoutnPosition->m_FrozenOvnQty, pAccoutnPosition->m_TodayQty, pAccoutnPosition->m_FrozenTodayQty,
            pAccoutnPosition->m_OvnMargin, pAccoutnPosition->m_TodayMargin);
    }
}

void CEESTrader::OnQueryTradeOrder(const char* pAccount, EES_QueryAccountOrder* pQueryOrder, bool bFinish)
{
    if(bFinish)
    {
        status_ = WAIT_QUERY;
    }
    else
    {
        const char *side = parseSide(pQueryOrder->m_SideType);
        const char *kind = NULL;
        switch(pQueryOrder->m_InstrumentType)
        {
        case EES_SecType_cs: kind = "股票"; break;
        case EES_SecType_options: kind = "期权"; break;
        case EES_SecType_fut: kind = "期货"; break;
        default: kind = "Unknown";
        }

        string status = parseOrderStatus(pQueryOrder->m_OrderStatus);
        printf("%-5d %-10d %-11u %-11lld %-10s   %-10u %-8s   %-18s %-12.2f %-10d %s\n", id_, pQueryOrder->m_Userid, pQueryOrder->m_ClientOrderToken, pQueryOrder->m_MarketOrderToken,
            side, pQueryOrder->m_Quantity, kind, pQueryOrder->m_symbol, pQueryOrder->m_Price, pQueryOrder->m_FilledQty, status.c_str());

        stringstream ss;
		ss << pQueryOrder->m_account << " " << pQueryOrder->m_MarketOrderToken;
		CGlobalParameter::order_id_map_[id_] = ss.str();
		id_++;

        if(id_inst_map_.find(make_pair(pQueryOrder->m_Userid, pQueryOrder->m_ClientOrderToken)) == id_inst_map_.end())
        {
            string &odinfo = id_inst_map_[make_pair(pQueryOrder->m_Userid, pQueryOrder->m_ClientOrderToken)];
            odinfo = pQueryOrder->m_symbol;
            odinfo += "|";
            odinfo += side;
        }
    }
}

void CEESTrader::OnQueryTradeOrderExec(const char* pAccount, EES_QueryOrderExecution* pQueryOrderExec, bool bFinish)
{
    if(bFinish)
    {
        status_ = WAIT_QUERY;
    }
    else
    {
        string inst("unknown");
        if(id_inst_map_.find(make_pair(pQueryOrderExec->m_Userid, pQueryOrderExec->m_ClientOrderToken)) != id_inst_map_.end())
        {
            inst = id_inst_map_[make_pair(pQueryOrderExec->m_Userid, pQueryOrderExec->m_ClientOrderToken)];
        }
        printf("%-10d %-11u %-11lld %-10u %-12.2f %s\n", pQueryOrderExec->m_Userid, pQueryOrderExec->m_ClientOrderToken, pQueryOrderExec->m_MarketOrderToken,
            pQueryOrderExec->m_ExecutedQuantity, pQueryOrderExec->m_ExecutionPrice, inst.c_str());
    }
}

void CEESTrader::OnQuerySymbol(EES_SymbolField* pSymbol, bool bFinish)
{
    if(bFinish)
    {
        status_ = WAIT_QUERY;
    }
    else
    {
        symbol_info_[pSymbol->m_symbol] = pSymbol->m_ExchangeID;
    }
}

void CEESTrader::OnOrderAccept(EES_OrderAcceptField* pAccept)
{
    string status = parseOrderStatus(pAccept->m_OrderState);
    printf("OnOrderAccept: UserID:%d ClientToken:%u MarketToken:%lld Inst:%s Side:%s Price:%.2lf Volume:%u Status:%s\n",
        pAccept->m_UserID, pAccept->m_ClientOrderToken, pAccept->m_MarketOrderToken, pAccept->m_Symbol, parseSide(pAccept->m_Side),
        pAccept->m_Price, pAccept->m_Qty, status.c_str());

    if(id_inst_map_.find(make_pair(pAccept->m_UserID, pAccept->m_ClientOrderToken)) == id_inst_map_.end())
    {
        stringstream ss;
        ss << pAccept->m_Symbol << "|" << parseSide(pAccept->m_Side) << "|" << pAccept->m_Price << "|" << pAccept->m_Qty;
        id_inst_map_[make_pair(pAccept->m_UserID, pAccept->m_ClientOrderToken)] = ss.str();
    }
}

void CEESTrader::OnOrderReject(EES_OrderRejectField* pReject)
{
    const char *reject_by = pReject->m_RejectedMan == 1 ? "盛立" : "交易所";
    string info = "*";
    if(id_inst_map_.find(make_pair(pReject->m_Userid, pReject->m_ClientOrderToken)) != id_inst_map_.end())
    {
        info = id_inst_map_[make_pair(pReject->m_Userid, pReject->m_ClientOrderToken)];
    }

    string grammer_msg = CEncodeConv::gbk2utf8(pReject->m_GrammerText);
    string risk_msg = CEncodeConv::gbk2utf8(pReject->m_RiskText);
    printf("OnOrderReject: UserID:%d ClientToken:%u Info:%s RejectedBy:%s ReasonCode:%d GrammerResult:%s RiskResult:%s\n", pReject->m_Userid,
        pReject->m_ClientOrderToken, info.c_str(), reject_by, pReject->m_ReasonCode, grammer_msg.c_str(), risk_msg.c_str());
}

void CEESTrader::OnOrderMarketAccept(EES_OrderMarketAcceptField* pAccept)
{
    printf("OnOrderMarketAccept: MarketToken:%lld\n", pAccept->m_MarketOrderToken);
}

void CEESTrader::OnOrderMarketReject(EES_OrderMarketRejectField* pReject)
{
    string msg = CEncodeConv::gbk2utf8(pReject->m_ReasonText);
    printf("OnOrderMarketReject: MarketToken:%lld Reason:%s\n", pReject->m_MarketOrderToken, msg.c_str());
}

void CEESTrader::OnOrderExecution(EES_OrderExecutionField* pExec)
{
    string info = "*";
    if(id_inst_map_.find(make_pair(pExec->m_Userid, pExec->m_ClientOrderToken)) != id_inst_map_.end())
    {
        info = id_inst_map_[make_pair(pExec->m_Userid, pExec->m_ClientOrderToken)];
    }
    printf("OnOrderExecution: UserID:%d ClientToken:%u MarketToken:%lld Info:%s ExecVolume:%u ExecPrice:%.2lf\n", pExec->m_Userid,
        pExec->m_ClientOrderToken, pExec->m_MarketOrderToken, info.c_str(), pExec->m_Quantity, pExec->m_Price);
}

void CEESTrader::OnOrderCxled(EES_OrderCxled* pCxled)
{
    string info = "*";
    if(id_inst_map_.find(make_pair(pCxled->m_Userid, pCxled->m_ClientOrderToken)) != id_inst_map_.end())
    {
        info = id_inst_map_[make_pair(pCxled->m_Userid, pCxled->m_ClientOrderToken)];
    }

    const char *reason = NULL;
    switch(pCxled->m_Reason)
    {
    case EES_CxlReasonCode_by_account: reason = "用户撤单"; break;
    case EES_CxlReasonCode_timeout: reason = "单子到期被交易所系统取消"; break;
    case EES_CxlReasonCode_supervisory: reason = "被盛立系统管理者取消"; break;
    case EES_CxlReasonCode_by_market: reason = "被市场拒绝"; break;
    case EES_CxlReasonCode_another: reason = "其他"; break;
    default: reason = "succ";
    }
    printf("OnOrderCxled: UserID:%d ClientToken:%u MarketToken:%lld Info:%s CxlVolume:%u Reason:%u|%s\n", pCxled->m_Userid,
        pCxled->m_ClientOrderToken, pCxled->m_MarketOrderToken, info.c_str(), pCxled->m_Decrement, pCxled->m_Reason, reason);
}

void CEESTrader::OnCxlOrderReject(EES_CxlOrderRej* pReject)
{
    string reason;
    if(1 & pReject->m_ReasonCode) reason += "整体校验结果;";
    if(2 & pReject->m_ReasonCode) reason += "委托尚未被交易所接受;";
    if(4 & pReject->m_ReasonCode) reason += "要撤销的委托找不到;";
    if(8 & pReject->m_ReasonCode) reason += "撤销的用户名和委托的用户名不一致;";
    if(16 & pReject->m_ReasonCode) reason += "撤销的账户和委托的账户不一致;";
    if(32 & pReject->m_ReasonCode) reason += "委托已经关闭，如已经撤销/成交等;";
    if(64 & pReject->m_ReasonCode) reason += "重复撤单;";
    if(128 & pReject->m_ReasonCode) reason += "被动单不能被撤单;";

    printf("OnCxlOrderReject: Account:%s MarketToken:%lld ReasonCode:%u Reason:%s\n",
        pReject->m_account, pReject->m_MarketOrderToken, pReject->m_ReasonCode, reason.c_str());
}

int CEESTrader::qryTradeMargin()
{
    for(uint32_t i = 0; i < accounts_.size(); ++i)
    {
        EES_AccountInfo & acc = accounts_[i];
        status_ = ON_QUERY;
        RESULT ret_err = trade_api_->QueryAccountTradeMargin(acc.m_Account);
        if (ret_err != NO_ERROR)
        {
            status_ = WAIT_QUERY;
            LOG_ERR("QueryTradeMargin %s failed!: %d\n", acc.m_Account, ret_err);
            continue;
        }
        while(ON_QUERY == status_) usleep(50000);
    }
    CGlobalParameter::terminal_lock_ = false;
    return 0;
}

void CEESTrader::OnQueryAccountTradeMargin(const char* pAccount, EES_AccountMargin* pSymbolMargin, bool bFinish)
{
    if(bFinish)
    {
        status_ = WAIT_QUERY;
    }
    else
    {
    	printf("Account:%s Symbol:%s LongMargin:%lf ShortMargin:%lf\n",
    			pAccount, pSymbolMargin->m_symbol, pSymbolMargin->m_LongMarginRatio, pSymbolMargin->m_ShortMarginRatio);
    }
}

int CEESTrader::qryTradeFee()
{
    for(uint32_t i = 0; i < accounts_.size(); ++i)
    {
        EES_AccountInfo & acc = accounts_[i];
        status_ = ON_QUERY;
        RESULT ret_err = trade_api_->QueryAccountTradeFee(acc.m_Account);
        if (ret_err != NO_ERROR)
        {
            status_ = WAIT_QUERY;
            LOG_ERR("QueryTradeFee %s failed!: %d\n", acc.m_Account, ret_err);
            continue;
        }
        while(ON_QUERY == status_) usleep(50000);
    }
    CGlobalParameter::terminal_lock_ = false;
    return 0;
}

void  CEESTrader::OnQueryAccountTradeFee(const char* pAccount, EES_AccountFee* pFee, bool bFinish)
{
    if(bFinish)
    {
        status_ = WAIT_QUERY;
    }
    else
    {
    	printf("Account:%s Symbol:%s OpenByM:%lf OpenByV:%lf CloseYDByM:%lf CloseYDByV:%lf CloseByM:%lf CloseByV:%lf\n",
    			pAccount, pFee->m_symbol, pFee->m_OpenRatioByMoney, pFee->m_OpenRatioByVolume, pFee->m_CloseYesterdayRatioByMoney,
				pFee->m_CloseYesterdayRatioByVolume, pFee->m_CloseTodayRatioByMoney, pFee->m_CloseTodayRatioByVolume);
    }
}


