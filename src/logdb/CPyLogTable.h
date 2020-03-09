/*
 * CPyLogTable.h
 *
 *  Created on: 2018年2月26日
 *      Author: hongxu
 */

#ifndef SRC_LOGDB_CPYLOGTABLE_H_
#define SRC_LOGDB_CPYLOGTABLE_H_

#include <boost/python.hpp>
#include "CLogTable.h"
using namespace boost::python;


class CPyLogTable: public CLogTable
{
public:
	CPyLogTable();
	virtual ~CPyLogTable();

	virtual bool parseLine(const char *p);

	list getValues() { return values_; }

	list getColNames()
	{
		list res;
		for(auto itr : select_col_name_) res.append(itr);
		return res;
	}

	list getColTypes()
	{
		list res;
		for(auto itr : select_col_types_)
		{
			switch(itr)
			{
			case COL_TYPE_STRING: res.append("string"); break;
			case COL_TYPE_INT: res.append("int"); break;
			case COL_TYPE_DOUBLE: res.append("double"); break;
			default: res.append("unknown");
			}
		}
		return res;
	}

private:
	list 	values_;
};

#endif /* SRC_LOGDB_CPYLOGTABLE_H_ */
