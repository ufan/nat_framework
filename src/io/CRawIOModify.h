/*
 * CRawIOModify.h
 *
 *  Created on: 2018年11月2日
 *      Author: hongxu
 */

#ifndef SRC_IO_CRAWIOMODIFY_H_
#define SRC_IO_CRAWIOMODIFY_H_

#include "CRawIOReader.h"

class CRawIOModify: public CRawIOReader
{
public:
	CRawIOModify();
	virtual ~CRawIOModify();

	bool loadFile(string path);
};

#endif /* SRC_IO_CRAWIOMODIFY_H_ */
