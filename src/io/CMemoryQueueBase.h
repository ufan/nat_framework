/*
 * CMemoryQueueBase.h
 *
 *  Created on: Sep 6, 2017
 *      Author: hongxu
 */

#ifndef LIB_MEMORYQUEUE_CMEMORYQUEUEBASE_H_
#define LIB_MEMORYQUEUE_CMEMORYQUEUEBASE_H_

#include <stdint.h>
#include <string>
#include <pthread.h>
#include <vector>

namespace IPC {

class CMemoryQueueBase
{
public:
	static const uint32_t MAX_QUEUE = 4096 * 2;
	static const uint32_t MAX_NAME_SIZE = 128;
	static const uint32_t MAGIC_SHMKEY = 0x20170906;

#pragma pack(1)
	struct stQueueInfo
	{
		char 		ipc_name_[MAX_NAME_SIZE];
		uint32_t 	shmkey_;
	};

	struct stQueueBase
	{
		uint32_t			reserve_;
		int 				queuecnt_;
		stQueueInfo 		queue_info[MAX_QUEUE];
	};
#pragma pack()


public:
	virtual ~CMemoryQueueBase() {}

private:
	CMemoryQueueBase();

public:
	static CMemoryQueueBase* instance();

	bool init(std::string filename);

	uint32_t getShmKeyByName(std::string ipc_name);

	uint32_t registerQueue(std::string name);

	void deleteQueue(std::string name);

	void list();

	void list(std::vector<std::string> &vques);

	bool tryFileLock(int off);

	bool setFileLock(int off);

	bool unsetFileLock(int off);

	bool globalLock(int off=0);

	bool globalUnlock(int off=0);

	bool tryLockKey(uint64_t key);

	bool unLockKey(uint64_t key);

protected:
	stQueueBase 			*pstart_;
	pthread_mutex_t			lock_;
	int						ifd_;
};



} /* namespace IPC */


#endif /* LIB_MEMORYQUEUE_CMEMORYQUEUEBASE_H_ */

