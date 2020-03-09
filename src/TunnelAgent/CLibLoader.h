/*
 * CLibLoader.h
 *
 *  Created on: 2017年10月4日
 *      Author: hongxu
 */

#ifndef SRC_TUNNELAGENT_CLIBLOADER_H_
#define SRC_TUNNELAGENT_CLIBLOADER_H_

#include "CFileReciever.h"
#include "ev.h"

class CLibLoader: public CFileReciever
{
	struct tChildWatcher
	{
		ev_child wathcer;
		string 	 name;
	};

public:
	CLibLoader(CWaiter *p_owner);
	virtual ~CLibLoader();

	int run(string &pkg);

	int load();

	static void child_cb(EV_P_ ev_child* w, int revents);

private:
	bool checkTmpDir(string dir_path);

	void runChild(string dir_path);

private:
	string 				conf_;
	CFileReciever		*p_conf_reciever_;
};

#endif /* SRC_TUNNELAGENT_CLIBLOADER_H_ */
