/*
 * CDirReciever.h
 *
 *  Created on: 2018年3月12日
 *      Author: hongxu
 */

#ifndef SRC_TA_CDIRRECIEVER_H_
#define SRC_TA_CDIRRECIEVER_H_

#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <memory>
#include "CCommanderBase.h"
#include "CFileReciever.h"
#include "protocol.h"
using namespace std;

class CDirReciever : public CCommanderBase
{
public:
	CDirReciever(CWaiter *p_owner);
	virtual ~CDirReciever();

	virtual int run(string &pkg);

	virtual int processPkg(string &pkg);

	virtual bool writeFile();

	virtual bool makedir();

	virtual string getDirPath() {return dirname_;}

protected:
	bool						is_first_pkg_;
	string						dirname_;
	unique_ptr<CFileReciever> 	p_file_reciever_;
};

#endif /* SRC_TA_CDIRRECIEVER_H_ */
