/*
 * CEncodeConv.h
 *
 *  Created on: 2018年1月5日
 *      Author: hongxu
 */

#ifndef SRC_TRADER_CENCODECONV_H_
#define SRC_TRADER_CENCODECONV_H_

#include <string>
using namespace std;

class CEncodeConv {
public:
	CEncodeConv();
	virtual ~CEncodeConv();

	static string conv(string str, string from_charset, string to_charset);

	static string gbk2utf8(string str) {return conv(str, "gbk", "utf8");}
};

#endif /* SRC_TRADER_CENCODECONV_H_ */
