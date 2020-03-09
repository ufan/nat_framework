/*
 * CGlobalLock.h
 *
 *  Created on: 2018年4月25日
 *      Author: hongxu
 */

#ifndef SRC_COMMON_CGLOBALLOCK_H_
#define SRC_COMMON_CGLOBALLOCK_H_

#include <stdint.h>
#include <atomic>
using namespace std;

class CGlobalLock
{
public:
	CGlobalLock();
	virtual ~CGlobalLock();

	bool init(const char *lockfile, int lock_file_size=4096);

	void lock(uint64_t hash);

	void unlock(uint64_t hash);

	bool trylock(uint64_t hash);

	bool isWork() {return fd_ >= 0;}

private:
	int 					fd_;
	int 					file_size_;
	atomic_flag 			flag_;
};


#endif /* SRC_COMMON_CGLOBALLOCK_H_ */
