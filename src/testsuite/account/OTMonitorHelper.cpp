#include "OTMonitorHelper.h"
#include "utils.h"
#include <string>
#include "ATStructure.h"
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>
#include <glob.h>
#include <stdlib.h>
using namespace std;

bool OTMonitorHelper::readOT(const char* path, int bgn, int end)
{
	int fd_ = open(path, O_RDONLY, (mode_t)0666);
	if (fd_ < 0)
	{
		printf("Cannot open file %s, err:%s\n", path, strerror(errno));
		return false;
	}

	struct stat statbuff;
	if(fstat(fd_, &statbuff) < 0)
	{
		close(fd_); fd_ = -1;
		printf("stat file %s fail, err:%s\n", path, strerror(errno));
		return false;
	}

	const uint32_t size = sizeof(tOrderTrackMmap);
	if(statbuff.st_size < size)
	{
		close(fd_); fd_ = -1;
		printf("order track mmap size mismatch.\n");
		return false;
	}

	tOrderTrackMmap* buf_ = (tOrderTrackMmap*)mmap(0, size, PROT_READ, MAP_SHARED, fd_, 0);
	if ((void*)buf_ == MAP_FAILED)
	{
		close(fd_); fd_ = -1;
		printf(" mapping file to buffer err:%s\n", strerror(errno));
		return false;
	}

	map_ot.clear();
//	printf("[ver]%u; [reserved]%lu; [trading_day]%s;\n", buf_->ver, buf_->reserved, buf_->trading_day);
	for (int i = bgn; i <= end && i < MMAP_ORDER_TRACK_SIZE; ++i)
	{
		tOrderTrack* p_ot = &buf_->order_track[i];
		if (p_ot->status != emOrderRtnType::UNKNOWN)
		{
//			printf("[status]%s; [instr_hash]%u; [instr]%s; [price]%.2lf; [vol]%d; [dir]%s; [off]%s; [vol_traded]%d; \
//[amount_traded]%.2lf; [from]%d; [local_id]%d; [acc_id]%d; [stg_id]%d; [order_ref]%ld; [front_id]%ld; [session_id]%ld;\n"
//				,getEmOrderRtnTypeString(p_ot->status).c_str()
//				,p_ot->instr_hash
//				,p_ot->instr
//				,p_ot->price
//				,p_ot->vol
//				,getDirString(p_ot->dir)
//				,getOffString(p_ot->off)
//				,p_ot->vol_traded
//				,p_ot->amount_traded
//				,p_ot->from
//				,p_ot->local_id
//				,p_ot->acc_id
//				,p_ot->stg_id
//				,p_ot->order_ref
//				,p_ot->front_id
//				,p_ot->session_id
//			);
			map_ot[p_ot->order_ref] = *p_ot;
		}
	}
}
