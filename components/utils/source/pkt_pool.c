
#include <stdint.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#include "dlist.h"
#include "pkt_pool.h"
#include "ks_os.h"
#include "ks_shell.h"



pbuf_t *pkt_pool_alloc(PktPoolCtx* pctx,int resv)
{
	pbuf_t *pkt;

	/**/
	pkt = (pbuf_t *)list_remove_head(&pctx->pkt_pool);
	if (pkt != NULL){

		--pctx->pktlist_statisticinfo.pkt_free_cnt;
		pctx->pktlist_statisticinfo.pkt_used_cnt++;
		if(pctx->pktlist_statisticinfo.pkt_used_cnt>pctx->pktlist_statisticinfo.pkt_maxused_cnt){
			pctx->pktlist_statisticinfo.pkt_maxused_cnt =pctx->pktlist_statisticinfo.pkt_used_cnt;
		}

		pkt->fun = NULL;
		pkt->offset = resv;
		pkt->length = 0;
	    pkt->tsize = pctx->pktlist_statisticinfo.pkt_buffer_size;
		
		pctx->pktlist_statisticinfo.pkt_malloc_times++;
	}

	return (pkt);
}



void pkt_pool_free(PktPoolCtx* pctx,pbuf_t *pkt)
{
	
	list_add_tail(&pctx->pkt_pool, (struct list_node *)pkt);
	
    ++pctx->pktlist_statisticinfo.pkt_free_cnt;
	pctx->pktlist_statisticinfo.pkt_used_cnt--;
	pctx->pktlist_statisticinfo.pkt_free_times++;

	return;
}


// 根据　 buffer指针(pbuf_t>buf)　获取　 pbuf_t　首地址　
int  pkt_pool_check(PktPoolCtx* pctx, void * buffer ,uint32_t* offset)
{
  	static  uintptr_t  temp;
    /**/
    temp = (uintptr_t)buffer;
    if ( temp < pctx->pktlist_statisticinfo.pkt_base )
    {
          return 1;
    }
    /**/
    if ( temp >= ( pctx->pktlist_statisticinfo.pkt_end ) )
    {
            return 2;
    }
    /**/
	temp = temp - pctx->pktlist_statisticinfo.pkt_base ;
	
    temp = temp % pctx->pktlist_statisticinfo.pkt_buffer_size;
    if ( temp != 0 )
    {
    	 *offset = temp;
         return 3;
    }
	
    return 0;

}


pbuf_t *  pkt_pool_align(  PktPoolCtx* pctx,uint8_t * pbuffer ){
	uint32_t offset;
	int ret = pkt_pool_check(pctx,pbuffer,&offset);

	if(ret==0)
	{
		return (pbuf_t*)pbuffer;
	}
	else if(ret==3)
	{
		return (pbuf_t*)((ubase_t)pbuffer-offset);

	}else{
		return (pbuf_t*)pbuffer;
	} 
}




int pkt_pool_init(int tnum, int buffersize,	uint8_t *ptr,PktPoolCtx* pctx)
{
	int i;


	/**/
	list_initialize(&pctx->pkt_pool);


	for (i = 0; i < tnum; i++)
	{
		list_add_tail(&pctx->pkt_pool, (struct list_node *)(ptr + (i * buffersize)));
	}

	pctx->pktlist_statisticinfo.pkt_base =(ubase_t) ptr;
	pctx->pktlist_statisticinfo.pkt_end = (ubase_t)(ptr + (tnum * buffersize));
    pctx->pktlist_statisticinfo.pkt_total_cnt = tnum;
    pctx->pktlist_statisticinfo.pkt_free_cnt = tnum;
	pctx->pktlist_statisticinfo.pkt_used_cnt= 0;
   	pctx->pktlist_statisticinfo.pkt_buffer_size = buffersize;
	

	return 0;
}




