/*
 * CEESOnload.cpp
 *
 *  Created on: 2018年2月7日
 *      Author: hongxu
 */

#include <stdio.h>
#include <iostream>
#include <stddef.h>
#include <inttypes.h>
#include <arpa/inet.h>
#include <net/if.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include "etherfabric/vi.h"
#include "etherfabric/pd.h"
#include "etherfabric/memreg.h"

#include "CEESOnload.h"
#include "Logger.h"


CEESOnload::CEESOnload() : is_run_(false), md_cb_(NULL)
{

}

CEESOnload::~CEESOnload()
{

}

bool CEESOnload::preparePktBuf()
{
    int bytes = N_RX_BUFS * BUF_SIZE;
    void* p;
    if(0 != posix_memalign(&p, 4096, bytes)) /* allocate aligned memory */
    {
        LOG_ERR("posix_memalign err:%s", strerror(errno));
        return false;
    }

    int ret = ef_memreg_alloc(&memreg_, driver_handle_, &pd_, driver_handle_, p, bytes); /* Make it available to ef_vi */
    if(ret < 0)
    {
        LOG_ERR("ef_memreg_alloc err:%d", ret);
        return false;
    }

    for(int i = 0; i < N_RX_BUFS; ++i)
    {
        struct pkt_buf* pb = (struct pkt_buf*) ((char*) p + i * BUF_SIZE);
        pb->id = i;
        pb->dma_buf_addr = ef_memreg_dma_addr(&memreg_, i * BUF_SIZE);
        pb->dma_buf_addr += offsetof(struct pkt_buf, dma_buf);
        pkt_bufs_[i] = pb;
    }

    for(int i = 0; i < N_RX_BUFS; ++i)
    {
        free_pkt_idx_[i] = N_RX_BUFS - i - 1;
    }
    free_pkt_cnt_ = N_RX_BUFS;

    return true;
}

bool CEESOnload::init(string interface, string ip, int port)
{
    int ret = ef_driver_open(&driver_handle_);
    if(ret < 0)
    {
        LOG_ERR("ef_driver_open err:%s, ret:%d", strerror(errno), ret);
        return false;
    }

    unsigned int ifindex;
    if( (ifindex = if_nametoindex(interface.c_str())) == 0 )
    {
        LOG_ERR("%s interface not found. err:%s", interface.c_str(), strerror(errno));
        return false;
    }

    ret = ef_pd_alloc(&pd_, driver_handle_, ifindex, EF_PD_DEFAULT);
    if(ret < 0)
    {
        LOG_ERR("ef_pd_alloc err:%d", ret);
        return false;
    }

    ret = ef_vi_alloc_from_pd(&vi_, driver_handle_, &pd_, driver_handle_, -1, -1, 0, NULL, -1, EF_VI_FLAGS_DEFAULT);
    if(ret < 0)
    {
        LOG_ERR("ef_vi_alloc_from_pd err:%d %s", ret, strerror(errno));
        return false;
    }

    if(!preparePktBuf()) return false;

    ef_filter_spec filter_spec;
    ef_filter_spec_init(&filter_spec, EF_FILTER_FLAG_NONE);
    ret = ef_filter_spec_set_ip4_local(&filter_spec, IPPROTO_UDP, inet_addr(ip.c_str()), htons((uint16_t)port));
    if(ret < 0)
    {
        LOG_ERR("ef_filter_spec_set_ip4_local err:%d", ret);
        return false;
    }

    ret = ef_vi_filter_add(&vi_, driver_handle_, &filter_spec, NULL);
    if(ret < 0)
    {
        LOG_ERR("ef_vi_filter_add err:%d", ret);
        return false;
    }

    return true;
}

inline void CEESOnload::refillRxRing()
{
    if( ef_vi_receive_space(&vi_) < REFILL_BATCH_SIZE || free_pkt_cnt_ < REFILL_BATCH_SIZE ) return;

    for( int i = 0; i < REFILL_BATCH_SIZE; ++i )
    {
        int idx = free_pkt_idx_[--free_pkt_cnt_];
        struct pkt_buf* p = pkt_bufs_[idx];
        ef_vi_receive_init(&vi_, p->dma_buf_addr, p->id);
    }
    ef_vi_receive_push(&vi_);
}

void CEESOnload::start()
{
	if(!md_cb_)
	{
		LOG_ERR("no md call back function.");
		return;
	}

	is_run_ = true;

    refillRxRing();

    ef_event evs[POLL_EVENT_COUNT];
    while(is_run_)
    {
        int n_ev = ef_eventq_poll(&vi_, evs, POLL_EVENT_COUNT);
        if(n_ev > 0)
        {
            for(int i = 0; i < n_ev; ++i)
            {
            	if(EF_EVENT_TYPE(evs[i]) == EF_EVENT_TYPE_RX)   // 单独写if，可以稍微提速
            	{
                    unsigned id = EF_EVENT_RX_RQ_ID(evs[i]);
                    md_cb_((guava_udp_normal*)((char*)(pkt_bufs_[id]->dma_buf) + HEADER_SIZE));
                    freePkt((int)id);
            	}
            	else
            	{
					switch( EF_EVENT_TYPE(evs[i]) )
					{
					case EF_EVENT_TYPE_RX_DISCARD:
					{
						/* Interesting to print out the cause of the discard */
						LOG_ERR("ERROR: RX_DISCARD type=%d", EF_EVENT_RX_DISCARD_TYPE(evs[i]));
						/* but let’s handle it like a normal packet anyway */

						unsigned id = EF_EVENT_RX_RQ_ID(evs[i]);
						md_cb_((guava_udp_normal*)((char*)(pkt_bufs_[id]->dma_buf) + HEADER_SIZE));
						freePkt((int)id);
						break;
					}
					default:
						LOG_ERR("ERROR: unexpected event " EF_EVENT_FMT "\n", EF_EVENT_PRI_ARG(evs[i]));
					}
            	}
            }
            refillRxRing();
        }
    }
}


