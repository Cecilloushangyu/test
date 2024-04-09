#ifndef __MEMPOOL_H__
#define __MEMPOOL_H__

#include <stdint.h>
#include "memblock.h"




typedef struct MemPoolCtx {
    int pool_cnt;
	MemPoolBlock* pool_block[20];
} MemPoolCtx;



typedef struct MemPoolStaticsInfo  {
    uint32_t totalCount;
    uint32_t usedCount;
    uint32_t unusedCount;
    uint32_t maxusedCount;
    uint32_t mallocCount;
    uint32_t freeCount;
    uint32_t failureCount;
}MemPoolStaticsInfo;



MemPoolCtx* MemPoolCtxGet();
int  MemPoolCreate(void * ptr, uint32_t buffer_size);
void * MemPoolMalloc( uint32_t size ,const char* callfuction  );
int  MemPoolFree( void *memory_ptr,const char* callfuction);
int MemPoolStatistics(MemPoolBlock* pmem, MemPoolStaticsInfo *info);



#endif
