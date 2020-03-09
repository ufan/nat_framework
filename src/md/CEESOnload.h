/*
 * CEESOnload.h
 *
 *  Created on: 2018年2月7日
 *      Author: hongxu
 */

#ifndef SRC_MDT_CEESONLOAD_H_
#define SRC_MDT_CEESONLOAD_H_

#include <string>
#include <stdint.h>

#include <etherfabric/vi.h>
#include <etherfabric/pd.h>
#include <etherfabric/memreg.h>

#include "efh_sf_api.h"

using namespace std;

typedef void (*EESMDCallBack)(const struct guava_udp_normal *p);

class CEESOnload
{
	static const uint32_t N_RX_BUFS = 256;
	static const uint32_t BUF_SIZE = 2048;
	static const uint32_t REFILL_BATCH_SIZE = 32;
	static const uint32_t POLL_EVENT_COUNT = 32;

	/* Protocol header length: Ethernet + IP + UDP. */
	static const uint32_t HEADER_SIZE = (14 + 20 + 8);

	struct pkt_buf
	{
	    struct pkt_buf* next;
	    ef_addr         dma_buf_addr;
	    int             id;
	    unsigned        dma_buf[1] EF_VI_ALIGN(EF_VI_DMA_ALIGN);
	};

public:
	CEESOnload();
	virtual ~CEESOnload();

	bool init(string interface, string ip, int port);

	void start();

	void stop() {is_run_ = false;}

	void registerMDCallBack(EESMDCallBack func) {md_cb_ = func;}

private:
	bool preparePktBuf();

	inline void freePkt(int id) { free_pkt_idx_[free_pkt_cnt_++] = id; }

	void refillRxRing();

private:
	bool				is_run_;
	ef_vi				vi_;
	EESMDCallBack		md_cb_;
	int             	free_pkt_cnt_;
	struct pkt_buf*     pkt_bufs_[N_RX_BUFS];
	int             	free_pkt_idx_[N_RX_BUFS];

	ef_driver_handle  	driver_handle_;
	ef_pd             	pd_;
	ef_memreg        	memreg_;
};

#endif /* SRC_MDT_CEESONLOAD_H_ */

