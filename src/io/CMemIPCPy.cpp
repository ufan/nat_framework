/*
 * CMemIPCPy.cpp
 *
 *  Created on: Sep 7, 2017
 *      Author: hongxu
 */

#include <boost/python.hpp>

#include "CMemoryQueueBase.h"
#include "CMemQueue.h"
#include "TimeMeasure.h"
using namespace IPC;
using namespace boost::python;

class CPyMemoryQueueBase
{
public:
	CPyMemoryQueueBase() {}
	~CPyMemoryQueueBase() {}

	bool init(std::string filename)
	{
		return CMemoryQueueBase::instance()->init(filename);
	}

	void list()
	{
		return CMemoryQueueBase::instance()->list();
	}
};

class CPyMemQueue : public CMemQueue
{
public:
	CPyMemQueue() {}
	~CPyMemQueue() {}

	void pywrite(std::string s)
	{
		write(s.data());
	}

	std::string pyread()
	{
		str_.resize(elemsize_);
		if(likely(read((void*)str_.data())))
		{
			return str_;
		}

		str_.clear();
		return str_;
	}

private:
	std::string str_;
};


uint64_t beginTimeMeasure()
{
	uint32_t high, low;
	getCycleBegin(high, low);
	return getu64(high, low);
}

uint64_t endTimeMeasure()
{
	uint32_t high, low;
	getCycleEnd(high, low);
	return getu64(high, low);
}

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(init_overloads, init, 3, 4)

BOOST_PYTHON_MODULE(pymemipc)
{
    class_<CPyMemoryQueueBase>("CMemoryQueueBase")
        .def("init", & CPyMemoryQueueBase::init)
		.def("list", & CPyMemoryQueueBase::list)
    ;

    class_<CPyMemQueue>("CMemQueue")
        .def("init", & CPyMemQueue::init, init_overloads())
		.def("destroy", & CPyMemQueue::destroy)
		.def("write", & CPyMemQueue::pywrite)
		.def("read", & CPyMemQueue::pyread)
		.def("resetReadSeq", & CPyMemQueue::resetReadSeq)
		.def("printInfo", & CPyMemQueue::printInfo)
    ;

    def("beginTimeMeasure", beginTimeMeasure);
    def("endTimeMeasure", endTimeMeasure);
}

