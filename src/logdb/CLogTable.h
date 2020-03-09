/*
 * CLogTable.h
 *
 *  Created on: 2018年2月9日
 *      Author: hongxu
 */

#ifndef SRC_LOGDB_CLOGTABLE_H_
#define SRC_LOGDB_CLOGTABLE_H_

#include <unordered_map>
#include <vector>
#include <string>
using namespace std;


enum emColType
{
	COL_TYPE_STRING,
	COL_TYPE_INT,
	COL_TYPE_DOUBLE,
	COL_TYPE_UNKNOWN,
};

class CLogTable
{
public:
	CLogTable();
	virtual ~CLogTable();

	void setColFilter(vector<string> &cols);

	vector<string> 	getColNames() { return select_col_name_; }

	vector<int>    	getColTypes() { return select_col_types_; }

	string 		   	getTableName() { return tbl_name_; }

	int				getColType(string col)
	{
		auto itr = select_col_map_.find(col);
		if(itr != select_col_map_.end())
		{
			return select_col_types_[itr->second];
		}
		return COL_TYPE_UNKNOWN;
	}

	const char*		getValBuffer(int line, string col)
	{
		auto itr = select_col_map_.find(col);
		if(itr != select_col_map_.end())
		{
			int pos = line * select_col_idx_.size() + itr->second;
			return select_col_value_[pos].c_str();
		}
		return NULL;
	}

	const char*		getValBuffer(int line, int col_idx)
	{
		int pos = line * select_col_idx_.size() + col_idx;
		return select_col_value_[pos].c_str();
	}

	int getLineCount()
	{
		return select_col_idx_.size() ? select_col_value_.size() / select_col_idx_.size() : 0;
	}

	void clear()
	{
		select_col_value_.clear();
		select_col_idx_.clear();
		select_col_types_.clear();
		select_col_name_.clear();
		tbl_name_.clear();
	}

	bool parseDescInfo(const char *p);

	// please skip the table name first
	virtual bool parseLine(const char *p);

protected:
	vector<string>					select_col_value_;
	vector<int>						select_col_idx_;
	vector<int>						select_col_types_;
	vector<string>					select_col_name_;
	unordered_map<string, int>		select_col_map_;
	string 							tbl_name_;
	bool							is_all_cols_;
};

#endif /* SRC_LOGDB_CLOGTABLE_H_ */
