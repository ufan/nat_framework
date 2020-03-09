/*
 * CLogDB.h
 *
 *  Created on: 2018年2月9日
 *      Author: hongxu
 */

#ifndef SRC_LOGDB_CLOGDB_H_
#define SRC_LOGDB_CLOGDB_H_

#include <string>
#include <unordered_set>
#include <memory>
#include <vector>
#include "CLogTable.h"
using namespace std;

// v1.0
class CLogDB
{
public:
	CLogDB();
	virtual ~CLogDB();

	void addFilter(string tbl, vector<string> cols);

	bool read(string file);

	CLogTable* getLogTable(string name)
	{
		auto itr = tbl_parser_.find(name);
		if(itr != tbl_parser_.end())
		{
			return itr->second.get();
		}
		return NULL;
	}

	vector<string> getTableNames();

	string getDBName() { return db_name_; }

	string getVer() { return version_; }

protected:
	virtual CLogTable* newLogTable() { return new CLogTable; }

	bool parseFirstLine(const char *p);

protected:
	bool 	has_filter_;
	unordered_map<string, shared_ptr<CLogTable> >  	tbl_parser_;
	string							  	db_name_;
	string							  	version_;
};

#endif /* SRC_LOGDB_CLOGDB_H_ */
