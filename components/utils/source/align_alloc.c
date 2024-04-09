
#include <stdint.h>
#include <stdio.h>

#include <string.h>
#include <errno.h>
#include "align_alloc.h"



void *align_alloc( align_alloc_ctx* pctx,size_t size, int align)
{
    ubase_t aptr;
    size_t asiz;
 	ubase_t  mptr = pctx->addr;
	size_t  msiz = pctx->freesize;
    /* 8 - 1024 : bytes align */
    if (align < 3)
    {
        align = 3;
    }
    else if (align > 16)
    {
        align = 16;
    }

    /* align --> mask */
    aptr = mptr;
    align = (1 << align) - 1;
    if (0 != (aptr & align))
    {
        aptr = aptr + align;
        aptr = aptr & (~align);
    }

    /* too large, not fit */
    asiz = (aptr - mptr) + size;
    if (asiz > msiz)
    {
    
        return NULL;
    }

    /**/
    mptr = aptr + size;
    msiz = msiz - asiz;
	
	pctx->addr = mptr;
	pctx->freesize = msiz;
    pctx->usedsize = pctx->usedsize + size;
    /**/
    return (void *)aptr;
}


int  align_init( align_alloc_ctx* pctx,ubase_t addr, size_t tsiz )
{
	pctx->addr = addr;
    pctx->totalsize = tsiz;
    pctx->usedsize= 0;
    pctx->freesize= tsiz;
	return 0;
}
