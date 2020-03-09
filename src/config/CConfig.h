/*
 * CConfig.h
 *
 *  Created on: Sep 21, 2017
 *      Author: hongxu
 */

#ifndef CONFIG_CCONFIG_H_
#define CONFIG_CCONFIG_H_

#include <sstream>
#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <fstream>
#include <boost/algorithm/string/trim.hpp>
#include <boost/algorithm/string/split.hpp>
using namespace std;


/*
 * ini-like config parser
 */
class CConfig
{
	typedef map<string, vector<string> >	keyval;
	typedef map<string, keyval> confmap;

public:
	struct FileNotFound		// exception throw by init
	{
		string file;
		FileNotFound(string file_) : file(file_) {}
	};

	struct KeyNotFound		// exception throw by getVal
	{
		string key;
		KeyNotFound(string key_) : key(key_) {}
	};

	CConfig() {}
	CConfig(string file)
	{
		if(!init(file)) throw FileNotFound(file);
	}
	virtual ~CConfig() {}

	inline bool initConfStr(string &config_content);

	inline bool init(string file);

	template<class T>
	T string_as_T(const string &s)
	{
		T t;
		istringstream ist(s);
		ist >> t;
		return t;
	}

	template<class T>
	void vector_as_T(const vector<string> &ori, vector<T> &res)
	{
		res.clear();
		for(vector<string>::const_iterator iter = ori.begin(); iter != ori.end(); ++iter)
		{
			res.push_back(string_as_T<T>(*iter));
		}
	}

	template<class T>
	bool getValList(string block, string key, vector<T> &res)
	{
		confmap::iterator iter = config_map_.find(block);
		if(iter !=  config_map_.end())
		{
			keyval::iterator it = iter->second.find(key);
			if(it != iter->second.end())
			{
				vector_as_T<T>(it->second, res);
				return true;
			}
		}
		return false;
	}

	template<class T>
	T getVal(string block, string key)
	{
		vector<string> vals;
		if( !getValList(block, key, vals) || vals.size() <= 0 )
		{
			throw KeyNotFound(key);
		}
		return string_as_T<T>(vals[0]);
	}

	template<class T>
	T getVal(string block, string key, const T &default_val)
	{
		vector<string> vals;
		if( !getValList(block, key, vals) || vals.size() <= 0 )
		{
			return default_val;
		}
		return string_as_T<T>(vals[0]);
	}

	template<class T>
	void getList(string block, string key, vector<T> &res, string delim=",")
	{
		vector<string> destination;
		string source = getVal<string>(block, key, "");
		boost::split(destination, source, boost::is_any_of(delim), boost::token_compress_on);
		vector_as_T(destination, res);
	}

protected:
	confmap  config_map_;
};

template<>
inline string CConfig::string_as_T<string>(const string &s)
{
	return s;
}

template<>
inline void CConfig::vector_as_T<string>(const vector<string> &ori, vector<string> &res)
{
	res = ori;
}

inline bool CConfig::initConfStr(string &config_content)
{
	istringstream in(config_content);
	int lineno = 0;
	string cur_block("");
	string cur_key("");
	string line;
	while(getline(in, line))
	{
		lineno++;
		boost::trim(line);
		if(line.length() <= 0) continue;

		// deal comment
		size_t pos = line.rfind('#');
		if(pos != string::npos)
		{
			line = line.substr(0, pos);
			boost::trim(line);
		}
		if(line.length() <= 0) continue;

		if(line[0] == '[')		// deal block
		{
			if(line[line.length() - 1] != ']')
			{
				cerr << "config format err, line " << lineno << endl;
				return false;
			}

			cur_block = line.substr(1, line.length() - 2);
			boost::trim(cur_block);
		}
		else	// deal key-value, support key=val, or key=val1 \n =val2 \n =val3 for list.
		{
			string val;
			size_t pos = line.find('=');
			if(pos != string::npos)
			{
				string key = line.substr(0, pos);
				boost::trim(key);
				if(key.size()) cur_key = key;

				val = line.substr(pos + 1);
				boost::trim(val);

				if(val.length() <= 0) continue;  // omit empty value
			}
			else
			{
				val = line;				// here can't be empty
			}

			keyval &blockmap = config_map_[cur_block];
			vector<string> &values = blockmap[cur_key];
			values.push_back(val);
		}
	}
	return true;
}

inline bool CConfig::init(string file)
{
	ifstream in(file.c_str());
	if(!in)
	{
		cerr << "config file " << file << " not exists." << endl;
		return false;
	}

	string content((std::istreambuf_iterator<char>(in)),
            std::istreambuf_iterator<char>());
	in.close();

	return initConfStr(content);
}


#endif /* CONFIG_CCONFIG_H_ */

