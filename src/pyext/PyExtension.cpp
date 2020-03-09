#include <boost/python.hpp>
#include "PyWrapper.h"
#include "CSignal.h"
#include "StrategyShared.h"
#include "RiskError.h"
#include "utils.h"

using namespace boost::python;

extern long getCurMdTime();
extern int getDailyCycle();
extern int getSessionCycle();
extern string getEmOrderRtnTypeString(int e);

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(CMDHelperComm_init_overloads, _init, 1, 3)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(ITDHelper_sendOrder_overloads, sendOrder, 6, 7)
BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(ITDHelperComm_init_overloads, _init, 1, 2)

BOOST_PYTHON_MODULE(libstrategy)
{
	def("getInstrHash", getInstrHash);
	def("getStgHash", getStgHash);
	def("exchangeint2str", exchangeint2str);

    class_<UnitedMarketData>("Tick")
		.def_readwrite("instr_hash", &UnitedMarketData::instr_hash)
		.def_readwrite("last_px", &UnitedMarketData::last_px)
		.def_readwrite("cum_vol", &UnitedMarketData::cum_vol)
		.def_readwrite("cum_turnover", &UnitedMarketData::cum_turnover)
		.def_readwrite("avg_px", &UnitedMarketData::avg_px)
		.def_readwrite("ask_px", &UnitedMarketData::ask_px)
		.def_readwrite("bid_px", &UnitedMarketData::bid_px)
		.def_readwrite("ask_vol", &UnitedMarketData::ask_vol)
		.def_readwrite("bid_vol", &UnitedMarketData::bid_vol)
		.def_readwrite("ask_px2", &UnitedMarketData::ask_px2)
		.def_readwrite("bid_px2", &UnitedMarketData::bid_px2)
		.def_readwrite("ask_vol2", &UnitedMarketData::ask_vol2)
		.def_readwrite("bid_vol2", &UnitedMarketData::bid_vol2)
		.def_readwrite("ask_px3", &UnitedMarketData::ask_px3)
		.def_readwrite("bid_px3", &UnitedMarketData::bid_px3)
		.def_readwrite("ask_vol3", &UnitedMarketData::ask_vol3)
		.def_readwrite("bid_vol3", &UnitedMarketData::bid_vol3)
		.def_readwrite("ask_px4", &UnitedMarketData::ask_px4)
		.def_readwrite("bid_px4", &UnitedMarketData::bid_px4)
		.def_readwrite("ask_vol4", &UnitedMarketData::ask_vol4)
		.def_readwrite("bid_vol4", &UnitedMarketData::bid_vol4)
		.def_readwrite("ask_px5", &UnitedMarketData::ask_px5)
		.def_readwrite("bid_px5", &UnitedMarketData::bid_px5)
		.def_readwrite("ask_vol5", &UnitedMarketData::ask_vol5)
		.def_readwrite("bid_vol5", &UnitedMarketData::bid_vol5)
		.def_readwrite("open_interest", &UnitedMarketData::open_interest)
		.def_readwrite("exch_time", &UnitedMarketData::exch_time)
		.add_property("instr_str", &UnitedMarketData::getInstrStr, &UnitedMarketData::setInstrStr)
        .def("__copy__", &generic__copy__< UnitedMarketData >)
        .def("__deepcopy__", &generic__deepcopy__< UnitedMarketData >)
    ;

	def("getDailyCycle", getDailyCycle);
	def("getSessionCycle", getSessionCycle);
    class_<Bar>("CBar")
		.def_readwrite("instr_hash", &Bar::instr_hash)
		.add_property("instr_str", &Bar::getInstrStr)
		.def_readwrite("cycle_sec", &Bar::cycle_sec)
		.def_readwrite("bob", &Bar::bob)
		.def_readwrite("eob", &Bar::eob)
		.def_readwrite("open", &Bar::open)
		.def_readwrite("high", &Bar::high)
		.def_readwrite("low", &Bar::low)
		.def_readwrite("close", &Bar::close)
		.def_readwrite("delta_close", &Bar::delta_close)
		.def_readwrite("vol", &Bar::vol)
		.def_readwrite("turnover", &Bar::turnover)
		.def_readwrite("delta_open_int", &Bar::delta_open_int)
		.def_readwrite("cum_vol", &Bar::cum_vol)
		.def_readwrite("cum_turnover", &Bar::cum_turnover)
		.def_readwrite("open_int", &Bar::open_int)
		.def_readwrite("is_auction", &Bar::is_auction)
	;

    class_<tLastTick>("LastTick")
		.def_readwrite("ask_px", &tLastTick::ask_px)
		.def_readwrite("bid_px", &tLastTick::bid_px)
		.def_readwrite("ask_vol", &tLastTick::ask_vol)
		.def_readwrite("bid_vol", &tLastTick::bid_vol)
		.def_readwrite("ask_px2", &tLastTick::ask_px2)
		.def_readwrite("bid_px2", &tLastTick::bid_px2)
		.def_readwrite("ask_vol2", &tLastTick::ask_vol2)
		.def_readwrite("bid_vol2", &tLastTick::bid_vol2)
		.def_readwrite("ask_px3", &tLastTick::ask_px3)
		.def_readwrite("bid_px3", &tLastTick::bid_px3)
		.def_readwrite("ask_vol3", &tLastTick::ask_vol3)
		.def_readwrite("bid_vol3", &tLastTick::bid_vol3)
		.def_readwrite("ask_px4", &tLastTick::ask_px4)
		.def_readwrite("bid_px4", &tLastTick::bid_px4)
		.def_readwrite("ask_vol4", &tLastTick::ask_vol4)
		.def_readwrite("bid_vol4", &tLastTick::bid_vol4)
		.def_readwrite("ask_px5", &tLastTick::ask_px5)
		.def_readwrite("bid_px5", &tLastTick::bid_px5)
		.def_readwrite("ask_vol5", &tLastTick::ask_vol5)
		.def_readwrite("bid_vol5", &tLastTick::bid_vol5)
		.def_readwrite("last_price", &tLastTick::last_price)
		.def_readwrite("cum_vol", &tLastTick::cum_vol)
        .def("__copy__", &generic__copy__< tLastTick >)
        .def("__deepcopy__", &generic__deepcopy__< tLastTick >)
	;

    enum_<emDir>("Dir")
    	.value("BUY", emDir::DIR_BUY)
		.value("SELL", emDir::DIR_SELL)
		.export_values()
	;

    enum_<emOffset>("Offset")
    	.value("OPEN", emOffset::OPEN)
		.value("CLOSE", emOffset::CLOSE)
		.value("FORCE_CLOSE", emOffset::FORCE_CLOSE)
		.value("CLOSE_TD", emOffset::CLOSE_TD)
		.value("CLOSE_YD", emOffset::CLOSE_YD)
		.value("FORCE_OFF", emOffset::FORCE_OFF)
		.value("LOCAL_FORCE_CLOSE", emOffset::LOCAL_FORCE_CLOSE)
		.value("NON", emOffset::NON)
		.value("AUTO", emOffset::AUTO)
		.export_values()
	;

    enum_<emOrderRtnType>("OrderRtnType")
    	.value("NOT_SET", emOrderRtnType::NOT_SET)
		.value("CLOSED", emOrderRtnType::CLOSED)
		.value("SEND", emOrderRtnType::SEND)
		.value("TDSEND", emOrderRtnType::TDSEND)
		.value("CXLING", emOrderRtnType::CXLING)
		.value("ACCEPT", emOrderRtnType::ACCEPT)
		.value("REJECT", emOrderRtnType::REJECT)
		.value("MARKET_ACCEPT", emOrderRtnType::MARKET_ACCEPT)
		.value("MARKET_REJECT", emOrderRtnType::MARKET_REJECT)
		.value("EXECUTION", emOrderRtnType::EXECUTION)
		.value("CANCEL_REJECT", emOrderRtnType::CANCEL_REJECT)
		.value("CANCELED", emOrderRtnType::CANCELED)
		.value("RTN_ERR", emOrderRtnType::ERR)
		.export_values()
	;

    class_<tOrderTrack>("OrderTrack")
		.def_readwrite("status", &tOrderTrack::status)
		.def_readwrite("instr_hash", &tOrderTrack::instr_hash)
		.add_property("instr_str", &tOrderTrack::getInstrStr, &tOrderTrack::setInstrStr)
		.def_readwrite("price", &tOrderTrack::price)
		.def_readwrite("vol", &tOrderTrack::vol)
		.def_readwrite("dir", &tOrderTrack::dir)
		.def_readwrite("off", &tOrderTrack::off)
		.def_readwrite("vol_traded", &tOrderTrack::vol_traded)
		.def_readwrite("amount_traded", &tOrderTrack::amount_traded)
		.def_readwrite("from", &tOrderTrack::from)
		.def_readwrite("local_id", &tOrderTrack::local_id)
		.def_readwrite("acc_id", &tOrderTrack::acc_id)
		.def_readwrite("stg_id", &tOrderTrack::stg_id)
		.def_readwrite("order_ref", &tOrderTrack::order_ref)
		.def_readwrite("front_id", &tOrderTrack::front_id)
		.def_readwrite("session_id", &tOrderTrack::session_id)
        .def("__copy__", &generic__copy__< tOrderTrack >)
        .def("__deepcopy__", &generic__deepcopy__< tOrderTrack >)
	;

    class_<tRtnMsg>("RtnMsg")
		.def_readwrite("msg_type", &tRtnMsg::msg_type)
		.def_readwrite("local_id", &tRtnMsg::local_id)
		.def_readwrite("instr_hash", &tRtnMsg::instr_hash)
		.add_property("instr", &tRtnMsg::getInstrStr, &tRtnMsg::setInstrStr)
		.def_readwrite("price", &tRtnMsg::price)
		.def_readwrite("vol", &tRtnMsg::vol)
		.def_readwrite("dir", &tRtnMsg::dir)
		.def_readwrite("off", &tRtnMsg::off)
		.def_readwrite("order_ref", &tRtnMsg::order_ref)
		.def_readwrite("front_id", &tRtnMsg::front_id)
		.def_readwrite("session_id", &tRtnMsg::session_id)
		.def_readwrite("errid", &tRtnMsg::errid)
		.add_property("msg", &tRtnMsg::getMsg, &tRtnMsg::setMsg)
        .def("__copy__", &generic__copy__< tRtnMsg >)
        .def("__deepcopy__", &generic__deepcopy__< tRtnMsg >)
    ;

    class_<tInstrumentInfo>("InstrumentInfo")
		.def_readwrite("instr_hash", &tInstrumentInfo::instr_hash)
		.add_property("instr", &tInstrumentInfo::getInstrStr, &tInstrumentInfo::setInstrStr)
		.def_readwrite("exch", &tInstrumentInfo::exch)
		.add_property("product", &tInstrumentInfo::getProduct, &tInstrumentInfo::setProduct)
		.def_readwrite("product_hash", &tInstrumentInfo::product_hash)
		.def_readwrite("vol_multiple", &tInstrumentInfo::vol_multiple)
		.def_readwrite("tick_price", &tInstrumentInfo::tick_price)
		.add_property("expire_date", &tInstrumentInfo::getExpireDate, &tInstrumentInfo::setExpireDate)
		.def_readwrite("is_trading", &tInstrumentInfo::is_trading)
        .def("__copy__", &generic__copy__< tInstrumentInfo >)
        .def("__deepcopy__", &generic__deepcopy__< tInstrumentInfo >)
    ;

    const UnitedMarketData* (IMDHelper::*md_read)() = &IMDHelper::read;
    class_<IMDHelper, boost::noncopyable>("IMDHelper", no_init)
    	.def("initStr", &IMDHelper::initStr)
		.def("read", md_read, return_internal_reference<>())
		.def("release", &IMDHelper::release)
		.def("setReadPos", &IMDHelper::setReadPos)
	;

    class_<CMDHelperCommWrapper, bases<IMDHelper> >("MDHelperComm", init<string>())
		.def("init", &CMDHelperCommWrapper::_init, CMDHelperComm_init_overloads())
		.def("subscribe", &CMDHelperCommWrapper::pySubscribe)
		.def("unsubscribe", &CMDHelperCommWrapper::pyUnsubscribe)
		.def("forceUnsubscribe", &CMDHelperCommWrapper::pyForceUnsubscribe)
		.def("getEngineSubscribedInstrument", &CMDHelperCommWrapper::pyGetEngineSubscribedInstrument)
    ;

    class_<CMDHelperReplayIOWrapper, bases<IMDHelper> >("MDHelperReplayIO", init<string>())
		.def("init", &CMDHelperReplayIOWrapper::_init)
		.def("next", &CMDHelperReplayIOWrapper::Next, return_internal_reference<>())
		.def("subscribe", &CMDHelperReplayIOWrapper::pySubscribe)
		.def("unsubscribe", &CMDHelperReplayIOWrapper::pyUnsubscribe)
		.def("getEngineSubscribedInstrument", &CMDHelperReplayIOWrapper::pyGetEngineSubscribedInstrument)
    ;

    int (ITDHelper::*td_sendOrder)(string, double, int, int, int, int, int) = &ITDHelper::sendOrder;
    class_<ITDHelper, boost::noncopyable>("ITDHelper", no_init)
		.def("initStr", &ITDHelper::initStr)
		.def("sendOrder", td_sendOrder, ITDHelper_sendOrder_overloads())
		.def("deleteOrder", &ITDHelper::deleteOrder)
		.def("getRtn", &ITDHelper::getRtn, return_internal_reference<>())
		.def("release", &ITDHelper::release)
		.def("on_tick", &ITDHelper::on_tick)
		.def("on_time", &ITDHelper::on_time)
		.def("getOrderTrackCnt", &ITDHelper::getOrderTrackCnt)
		.def("getOrderTrack", &ITDHelper::getOrderTrack, return_internal_reference<>())
	;

    class_<CTDHelperComm, bases<ITDHelper> >("TDHelperComm", init<string>())
		.def("init", &CTDHelperComm::_init, ITDHelperComm_init_overloads())
	;

    class_<CTDHelperFake, bases<ITDHelper> >("TDHelperFake", init<string>())
	;

    int (CStrategy::*stg_sendOrder)(string, double, int, int, int, int) = &CStrategy::sendOrder;
    int (CStrategy::*stg_sendOrder_hash)(uint32_t, double, int, int, int, int) = &CStrategy::sendOrder;
    const tLastTick* (CStrategy::*stg_getLastTick_hash)(uint32_t) = &CStrategy::getLastTick;
    const tLastTick* (CStrategy::*stg_getLastTick_str)(string) = &CStrategy::getLastTick;
    class_<CStrategy, boost::noncopyable>("SimpleStrategyBase")
		.def("init", &CStrategy::init)
		.def("initStr", &CStrategy::initStr)
		.def("run", &CStrategy::run)
		.def("stop", &CStrategy::stop)
		.def("release", &CStrategy::release)
		.def("subscribe", &CStrategy::subscribe)
		.def("subsBar", &CStrategy::subsBar)
		.def("setIntraDayBaseSec", &CStrategy::setIntraDayBaseSec)
		.def("sendOrder", stg_sendOrder)
		.def("sendOrder", stg_sendOrder_hash)
		.def("cancelOrder", &CStrategy::cancelOrder)
		.def("setReadPos", &CStrategy::setReadPos)
		.def("readMd", &CStrategy::readMd, return_internal_reference<>())
		.def("getLastTick", stg_getLastTick_hash, return_internal_reference<>())
		.def("getLastTick", stg_getLastTick_str, return_internal_reference<>())
		.def("getOrderTrackCnt", &CStrategy::getOrderTrackCnt)
		.def("getOrderTrack", &CStrategy::getOrderTrack, return_internal_reference<>())
		.def("getTradingDay", &CStrategy::getTradingDay)
	;

    class_<CPyStrategy, bases<CStrategy>, boost::noncopyable>("SimpleStrategy")
		.def("on_tick", &CPyStrategy::on_tick)
		.def("on_time", &CPyStrategy::on_time)
		.def("on_rtn", &CPyStrategy::on_rtn)
		.def("on_bar", &CPyStrategy::on_bar)
		.def("on_switch_day", &CPyStrategy::on_switch_day)
		.def("on_msg", &CPyStrategy::on_msg)
		.def("getAccObj", &CPyStrategy::getAccObj, return_internal_reference<>())
	;

    int (CStrategyBase::*base_sendOrder)(string, double, int, int, int, int) = &CStrategyBase::sendOrder;
    int (CStrategyBase::*base_sendOrder_hash)(uint32_t, double, int, int, int, int) = &CStrategyBase::sendOrder;
    class_<CPyStrategyBase, boost::noncopyable>("StrategyBase")
		.def("activate", &CStrategyBase::activate)
		.def("subscribe", &CStrategyBase::subscribe)
		.def("subsBar", &CStrategyBase::subsBar)
		.def("sendOrder", base_sendOrder)
		.def("sendOrder", base_sendOrder_hash)
		.def("cancelOrder", &CStrategyBase::cancelOrder)
		.def("on_tick", &CPyStrategyBase::on_tick)
		.def("on_time", &CPyStrategyBase::on_time)
		.def("on_rtn", &CPyStrategyBase::on_rtn)
		.def("on_bar", &CPyStrategyBase::on_bar)
		.def("on_switch_day", &CPyStrategyBase::on_switch_day)
		.def("getAccObj", &CPyStrategyBase::getAccObj, return_internal_reference<>())
	;

    const tLastTick* (*proc_getLastTick_hash)(uint32_t) = &CStrategyProcess::getLastTick;
    const tLastTick* (*proc_getLastTick_str)(string) = &CStrategyProcess::getLastTick;
    class_<CStrategyProcess, boost::noncopyable>("StrategyProcess")
		.def("init", &CStrategyProcess::init).staticmethod("init")
		.def("initStr", &CStrategyProcess::initStr).staticmethod("initStr")
		.def("run", &CStrategyProcess::run).staticmethod("run")
		.def("stop", &CStrategyProcess::stop).staticmethod("stop")
		.def("release", &CStrategyProcess::release).staticmethod("release")
		.def("setReadPos", &CStrategyProcess::setReadPos).staticmethod("setReadPos")
		.def("getLastTick", proc_getLastTick_hash, return_internal_reference<>())
		.def("getLastTick", proc_getLastTick_str, return_internal_reference<>()).staticmethod("getLastTick")
		.def("getOrderTrackCnt", &CStrategyProcess::getOrderTrackCnt).staticmethod("getOrderTrackCnt")
		.def("getOrderTrack", &CStrategyProcess::getOrderTrack, return_internal_reference<>()).staticmethod("getOrderTrack")
		.def("getTradingDay", &CStrategyProcess::getTradingDay).staticmethod("getTradingDay")
	;

	class_<CSignalReader>("SignalReader", init<string>())
		.def("read", &CSignalReader::readStr)
		.def("setReadPos", &CSignalReader::setReadPos)
	;

	class_<CSignalWriter>("SignalWriter", init<string>())
		.def("write", &CSignalWriter::writeStr)
	;

    def("getInstrumentInfo", getInstrumentInfo);
    def("getInstrumentInfoByHash", &CTradeBaseInfo::getInstrInfo, return_internal_reference<>());
    def("getTradingDay", CTradeBaseInfo::getTradingDay);
    def("setTradingDay", CTradeBaseInfo::setTradingDay);
    def("setTradeBaseInitFlag", CTradeBaseInfo::setInitFlag);
    def("addInstrInfo", CTradeBaseInfo::addInstrInfo);
    def("clearInstrInfo", CTradeBaseInfo::clearInstrInfo);
    def("callSysOnSwitchDayCb", CTradeBaseInfo::callOnSwitchDayCb);

	def("strError", &RiskError::strerror);
	def("getNewLogger", getNewLogger);
	def("fastLog", fastLog);
	def("getStgLoggerId", getStgLoggerId);
	def("initFastLoggerConf", initFastLoggerConf);
	
	class_<UnitVol>("UnitVol")
		.def_readwrite("pos_long_yd_ini", &UnitVol::pos_long_yd_ini)
		.def_readwrite("pos_long_yd_cls_ing", &UnitVol::pos_long_yd_cls_ing)
		.def_readwrite("pos_long_yd", &UnitVol::pos_long_yd)
		.def_readwrite("pos_long_td_opn_ing", &UnitVol::pos_long_td_opn_ing)
		.def_readwrite("pos_long_td_cls_ing", &UnitVol::pos_long_td_cls_ing)
		.def_readwrite("pos_long_td", &UnitVol::pos_long_td)
		.def_readwrite("pos_long", &UnitVol::pos_long)
		.def_readwrite("pos_short_yd_ini", &UnitVol::pos_short_yd_ini)
		.def_readwrite("pos_short_yd_cls_ing", &UnitVol::pos_short_yd_cls_ing)
		.def_readwrite("pos_short_yd", &UnitVol::pos_short_yd)
		.def_readwrite("pos_short_td_opn_ing", &UnitVol::pos_short_td_opn_ing)
		.def_readwrite("pos_short_td_cls_ing", &UnitVol::pos_short_td_cls_ing)
		.def_readwrite("pos_short_td", &UnitVol::pos_short_td)
		.def_readwrite("pos_short", &UnitVol::pos_short)
		.def_readwrite("pos_buy", &UnitVol::pos_buy)
		.def_readwrite("pos_sell", &UnitVol::pos_sell)
	;
	
	class_<UnitAmt>("UnitAmt")
		.def_readwrite("amt_long_yd_sub_ing", &UnitAmt::amt_long_yd_sub_ing)
		.def_readwrite("amt_long_td_add_ing", &UnitAmt::amt_long_td_add_ing)
		.def_readwrite("amt_long_td_sub_ing", &UnitAmt::amt_long_td_sub_ing)
		.def_readwrite("amt_long", &UnitAmt::amt_long)
		.def_readwrite("amt_short_yd_sub_ing", &UnitAmt::amt_short_yd_sub_ing)
		.def_readwrite("amt_short_td_add_ing", &UnitAmt::amt_short_td_add_ing)
		.def_readwrite("amt_short_td_sub_ing", &UnitAmt::amt_short_td_sub_ing)
		.def_readwrite("amt_short", &UnitAmt::amt_short)
		.def_readwrite("amt_buy", &UnitAmt::amt_buy)
		.def_readwrite("amt_sell", &UnitAmt::amt_sell)
	;
	
	class_<UnitPx>("UnitPx")
		.def_readwrite("stl_px_yd", &UnitPx::stl_px_yd)
		.def_readwrite("last_px", &UnitPx::last_px)
		.def_readwrite("avg_px_buy", &UnitPx::avg_px_buy)
		.def_readwrite("avg_px_sell", &UnitPx::avg_px_sell)
	;
	
	class_<UnitPnl>("UnitPnl")
		.def_readwrite("pos_pnl", &UnitPnl::pos_pnl)
		.def_readwrite("trd_pnl", &UnitPnl::trd_pnl)
	;
	
	class_<ModAcc>("ModAcc")
		.def("onSwitchDay", &ModAcc::onSwitchDay)
		.def_readwrite("unfilled_order_cnt", &ModAcc::unfilled_order_cnt)
		.def_readwrite("unit_amt", &ModAcc::unit_amt)
		.def_readwrite("unit_pnl", &ModAcc::unit_pnl)
	;
	
	class_<ModInstr>("ModInstr")
		.def("onSwitchDay", &ModInstr::onSwitchDay)
		.def("onTickPx", &ModInstr::onTickPx)
		.def("onNew", &ModInstr::onNew)
		.def("onCxl", &ModInstr::onCxl)
		.def("onTrd", &ModInstr::onTrd)
		.def_readwrite("instr_name", &ModInstr::getInstrName)
		.def_readwrite("vol_multiple", &ModInstr::vol_multiple)
		.def_readwrite("unit_vol", &ModInstr::unit_vol)
		.def_readwrite("unit_amt", &ModInstr::unit_amt)
		.def_readwrite("unit_px", &ModInstr::unit_px)
		.def_readwrite("unit_pnl", &ModInstr::unit_pnl)
	;
	
	class_<ModPrd>("ModPrd")
		.def("onSwitchDay", &ModPrd::onSwitchDay)
		.def_readwrite("prd_name", &ModPrd::getPrdName)
		.def_readwrite("unit_vol", &ModPrd::unit_vol)
		.def_readwrite("unit_amt", &ModPrd::unit_amt)
		.def_readwrite("unit_pnl", &ModPrd::unit_pnl)
	;
	
	class_<AccBaseWrapper>("AccBase")
		.def("init", &AccBaseWrapper::init)
		.def("regInstr", &AccBase::regInstr)
		.def("onSwitchDay", &AccBase::onSwitchDay)
		.def("save", &AccBase::save)
		.def("load", &AccBase::load)
		.def("onOrderTrack", &AccBase::onOrderTrack)
		.def("onNew", &AccBase::onNew)
		.def("onRtn", &AccBase::onRtn)
		.def("onTickPx", &AccBase::onTickPx)
		.def("getModAcc", &AccBase::getModAcc, return_internal_reference<>())
		.def("getModInstr", &AccBase::getModInstr, return_internal_reference<>())
		.def("getModPrd", &AccBase::getModPrd, return_internal_reference<>())
		.def("getAllPrd", &AccBaseWrapper::pyGetAllPrd)
		.def("getAllInstr", &AccBaseWrapper::pyGetAllInstr)
		.def("getAccUnitAmt", &AccBase::getAccUnitAmt, return_internal_reference<>())
		.def("getAccUnitPnl", &AccBase::getAccUnitPnl, return_internal_reference<>())
		.def("getInstrUnitVol", &AccBase::getInstrUnitVol, return_internal_reference<>())
		.def("getInstrUnitAmt", &AccBase::getInstrUnitAmt, return_internal_reference<>())
		.def("getInstrUnitPx", &AccBase::getInstrUnitPx, return_internal_reference<>())
		.def("getInstrUnitPnl", &AccBase::getInstrUnitPnl, return_internal_reference<>())
		.def("getPrdUnitVol", &AccBase::getPrdUnitVol, return_internal_reference<>())
		.def("getPrdUnitAmt", &AccBase::getPrdUnitAmt, return_internal_reference<>())
		.def("getPrdUnitPnl", &AccBase::getPrdUnitPnl, return_internal_reference<>())
	;
	
	class_<RiskStgWrapper>("RiskStg")
		.def("getModAcc", &RiskStg::getModAcc, return_internal_reference<>())
		.def("getModInstr", &RiskStg::getModInstr, return_internal_reference<>())
		.def("getModPrd", &RiskStg::getModPrd, return_internal_reference<>())
		.def("getAllPrd", &RiskStgWrapper::pyGetAllPrd)
		.def("getAllInstr", &RiskStgWrapper::pyGetAllInstr)
		.def("onTickPx", &RiskStg::onTickPx)
		.def("getAccUnitAmt", &RiskStg::getAccUnitAmt, return_internal_reference<>())
		.def("getAccUnitPnl", &RiskStg::getAccUnitPnl, return_internal_reference<>())
		.def("getInstrUnitVol", &RiskStg::getInstrUnitVol, return_internal_reference<>())
		.def("getInstrUnitAmt", &RiskStg::getInstrUnitAmt, return_internal_reference<>())
		.def("getInstrUnitPx", &RiskStg::getInstrUnitPx, return_internal_reference<>())
		.def("getInstrUnitPnl", &RiskStg::getInstrUnitPnl, return_internal_reference<>())
		.def("getPrdUnitVol", &RiskStg::getPrdUnitVol, return_internal_reference<>())
		.def("getPrdUnitAmt", &RiskStg::getPrdUnitAmt, return_internal_reference<>())
		.def("getPrdUnitPnl", &RiskStg::getPrdUnitPnl, return_internal_reference<>())
	;
	
	def("getEmOrderRtnTypeString", getEmOrderRtnTypeString);

	def("getStrategyConfig", getStrategyConfig);

	def("getCurMdTime", getCurMdTime);

	class_<CPyWareHouseReader, boost::noncopyable>("WareHouseReader")
		.def("openFile", &CPyWareHouseReader::openFile)
		.def("loadFileList", &CPyWareHouseReader::loadFileList)
		.def("loadFiles", &CPyWareHouseReader::pyLoadFiles)
		.def("readTick", &CPyWareHouseReader::readTick, return_internal_reference<>())
		.def("clear", &CPyWareHouseReader::clear)
		.def("doSetTimer", &CPyWareHouseReader::doSetTimer)
		.def("readOne", &CPyWareHouseReader::readOne)
		.def("read", &CPyWareHouseReader::read)
		.def("setReadPos", &CPyWareHouseReader::setReadPos)
	;

	def("setSystemTimeNano", setSystemTimeNano);

	class_<CPyRtnReader, boost::noncopyable>("RtnReader")
		.def("init", &CPyRtnReader::init)
		.def("getRtn", &CPyRtnReader::getRtn)
	;
}
