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

/*
 * CGlobalLock implements a lock mechanism based on the file write lock in Linux.
 * A lock file is created and each byte in the file can be used to lock one
 * resource.
 * The class is also thread-safe, i.e. it also solves data race conflict in multi-threading.
 * The typical usage of CGlobalLock is a static data member of the IOWriter class.
 */

class CGlobalLock
{
public:
	CGlobalLock();
	virtual ~CGlobalLock();

	bool init(const char *lockfile, int lock_file_size=4096);

  // lock the resource represented by 'hash'
	void lock(uint64_t hash);

  // unlock the resource represented by 'hash'
	void unlock(uint64_t hash);

  // try to lock the resource represented by 'hash'
	bool trylock(uint64_t hash);

  // whether lockfile created
	bool isWork() {return fd_ >= 0;}

private:
	int 					fd_;
	int 					file_size_;
	atomic_flag 			flag_;
};


#endif /* SRC_COMMON_CGLOBALLOCK_H_ */
