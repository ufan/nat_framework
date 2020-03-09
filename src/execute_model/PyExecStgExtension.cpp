/*
 * PyExecStgExtension.cpp
 *
 *  Created on: 2018年12月11日
 *      Author: hongxu
 */

#include "CExecuteStrategy.h"
#include "PyWrapper.h"
using namespace boost::python;

class CPyExecuteStrategy : public CExecuteStrategy, public bp::wrapper<CPyExecuteStrategy>
{
public:
	virtual void on_tick(const UnitedMarketData* pmd) {if (bp::override f = this->get_override("on_tick")) f(pmd);}
	virtual void on_time(long nano) {if (bp::override f = this->get_override("on_time")) f(nano);}
	virtual void on_rtn(const tRtnMsg* prtn) {if (bp::override f = this->get_override("on_rtn")) f(prtn);}
	virtual void on_bar(const Bar* pbar) {if (bp::override f = this->get_override("on_bar")) f(pbar);}
	virtual void on_switch_day(string day) {if (bp::override f = this->get_override("on_switch_day")) f(day);}
	virtual void on_msg(const char *p, uint32_t len) {if (bp::override f = this->get_override("on_msg")) f(bp::str(p, len));}
	const RiskStgWrapper* getAccObj() {return (const RiskStgWrapper*)getAccountObj();}
};

BOOST_PYTHON_MODULE(libexecstg)
{
	class_<CPyExecuteStrategy, bases<CStrategy>, boost::noncopyable>("ExecStrategyBase")
		.def("execution", &CPyExecuteStrategy::execution)
		.def("cancelExec", &CPyExecuteStrategy::cancelExec)
		.def("on_tick", &CPyExecuteStrategy::on_tick)
		.def("on_time", &CPyExecuteStrategy::on_time)
		.def("on_rtn", &CPyExecuteStrategy::on_rtn)
		.def("on_bar", &CPyExecuteStrategy::on_bar)
		.def("on_switch_day", &CPyExecuteStrategy::on_switch_day)
		.def("on_switch_day", &CPyExecuteStrategy::on_msg)
		.def("getAccObj", &CPyExecuteStrategy::getAccObj, return_internal_reference<>())
	;
}



