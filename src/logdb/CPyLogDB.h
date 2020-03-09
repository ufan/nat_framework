/*
 * CPyLogDB.h
 *
 *  Created on: 2018年2月26日
 *      Author: hongxu
 */

#ifndef SRC_LOGDB_CPYLOGDB_H_
#define SRC_LOGDB_CPYLOGDB_H_

#include "CLogDB.h"
#include "CPyLogTable.h"

class CPyLogDB: public CLogDB
{
public:
	CPyLogDB();
	virtual ~CPyLogDB();

	CPyLogTable& getTable(string name)
	{
		auto itr = tbl_parser_.find(name);
		if(itr != tbl_parser_.end())
		{
			return *(CPyLogTable*)itr->second.get();
		}
		return *(CPyLogTable*)NULL;
	}

	list getTableList()
	{
		list res;
		for(auto itr : tbl_parser_) res.append(itr.first);
		return res;
	}

	void addFilter(string tbl, list cols)
	{
		vector<string> vec;
		for(int i = 0; i < len(cols); ++i) vec.emplace_back(extract<string>(cols[i]));
		CLogDB::addFilter(tbl, vec);
	}

protected:
	virtual CLogTable* newLogTable() { return new CPyLogTable; }
};

#endif /* SRC_LOGDB_CPYLOGDB_H_ */
