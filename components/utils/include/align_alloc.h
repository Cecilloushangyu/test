
#pragma  once
#include <ks_datatypes.h>

typedef struct  align_alloc_ctx
{
	ubase_t addr;
    size_t   totalsize;
    size_t   usedsize;
    size_t   freesize;
}align_alloc_ctx;

//　size：大小　　　align　： 2 的　 align　次方对齐
void *align_alloc( align_alloc_ctx* pctx,size_t size, int align);
int  align_init( align_alloc_ctx* pctx,ubase_t addr, size_t tsiz );

