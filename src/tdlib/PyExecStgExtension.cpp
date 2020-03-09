/*
 * PyExecStgExtension.cpp
 *
 *  Created on: 2018年12月11日
 *      Author: hongxu
 */

#include <boost/python.hpp>
using namespace boost::python;

BOOST_PYTHON_MODULE(libexecstg)
{
    class_<CPyStrategy, boost::noncopyable>("ExecStrategy")
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
		.def("on_tick", &CPyStrategy::on_tick)
		.def("on_time", &CPyStrategy::on_time)
		.def("on_rtn", &CPyStrategy::on_rtn)
		.def("on_bar", &CPyStrategy::on_bar)
		.def("on_switch_day", &CPyStrategy::on_switch_day)
		.def("getOrderTrackCnt", &CStrategy::getOrderTrackCnt)
		.def("getOrderTrack", &CStrategy::getOrderTrack, return_internal_reference<>())
		.def("getTradingDay", &CStrategy::getTradingDay)
		.def("getAccObj", &CPyStrategy::getAccObj, return_internal_reference<>())
	;
}



