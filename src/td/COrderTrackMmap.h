/*
 * COrderMmap.h
 *
 *  Created on: 2018年6月3日
 *      Author: sky
 */

#ifndef TD_CORDERTRACKMMAP_H_
#define TD_CORDERTRACKMMAP_H_

#include <string>
#include "ATStructure.h"
using namespace std;

class COrderTrackMmap
{
public:
	COrderTrackMmap(bool lockmem=false);
	virtual ~COrderTrackMmap();

	bool load(string name, bool is_write=false);
	void unload();

	tOrderTrackMmap* getBuf() {return buf_;}

	void clearTrack();

protected:
	tOrderTrackMmap 	*buf_ 		= nullptr;
	int					fd_			= -1;
	bool				is_lock_    = false;
};

#endif /* TD_CORDERTRACKMMAP_H_ */
