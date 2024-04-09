
#pragma  once

#include <stdint.h>
#include <stddef.h>
#include <ks_datatypes.h>

typedef struct _tag_pbuf
{
	struct _tag_pbuf * next;
	void (* fun)(void *arg);
	int  offset;
	uint16_t  length;
	uint16_t  tsize;
	uint8_t  buf[0];
	
} pbuf_t;

typedef struct  PktPoolStatisticInfo
{
	ubase_t pkt_base;
	ubase_t pkt_end;
	uint32_t pkt_total_cnt;
	uint32_t pkt_free_cnt;
	uint32_t pkt_used_cnt;
	uint32_t pkt_maxused_cnt;
	uint32_t pkt_buffer_size;
	uint32_t pkt_free_times;
	uint32_t pkt_malloc_times;

}PktPoolStatisticInfo;



typedef struct  PktPoolCtx
{
	struct list_node pkt_pool;
	PktPoolStatisticInfo pktlist_statisticinfo;
}PktPoolCtx;


static inline uint8_t * pkt_to_addr( pbuf_t * pkt )
{
	return &( pkt->buf[pkt->offset] );
}


static inline uint8_t * pkt_prepend( pbuf_t * pkt, int tlen )
{
	if ( (tlen <= 0) || (tlen > pkt->offset) )
	{
		return NULL;
	}
	
	/**/
	pkt->offset -= tlen;
	pkt->length += tlen;
	return &( pkt->buf[pkt->offset] );
}


static inline uint8_t * pkt_append( pbuf_t * pkt, int tlen )
{
	uint8_t * ptr;

	/**/
	if ( (tlen <= 0) || (tlen > pkt->tsize) )
	{
		return NULL;
	}

	if ( (pkt->offset + pkt->length + tlen) > pkt->tsize )
	{
		return NULL;
	}

	/**/
	ptr = &( pkt->buf[pkt->offset + pkt->length] );
	pkt->length += tlen;
	return ptr;
	
}


static inline uint8_t * pkt_clip( pbuf_t * pkt, int tlen )
{
	if ( tlen <= 0 )
	{
		return NULL;
	}

	if ( pkt->length < tlen )
	{
		return NULL;
	}

	/**/
	pkt->offset += tlen;
	pkt->length -= tlen;
	return &( pkt->buf[pkt->offset] );
}


static inline int pkt_trim( pbuf_t * pkt, int tlen )
{
	if ( tlen <= 0 )
	{
		return 1;
	}

	if ( pkt->length < tlen )
	{
		return 2;
	}	
	
	/**/
	pkt->length -= tlen;
	return 0;
}

static inline int pkt_tail_add( pbuf_t * head, pbuf_t *pbuf)
{
	pbuf_t *p;

	if(head == NULL || pbuf == NULL)
	{
		return 1;
	}
	
	for(p= head; p->next != NULL; p=p->next);
	p->next = pbuf;
	return 0;
}



int pkt_pool_init(int tnum, int buffersize,	uint8_t *ptr,PktPoolCtx* pctx);
pbuf_t *pkt_pool_alloc(PktPoolCtx* pctx,int resv);
void pkt_pool_free(PktPoolCtx* pctx,pbuf_t *pkt);


