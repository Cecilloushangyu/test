


#pragma once


#ifdef __cplusplus
extern "C" {
#endif
#include <stdint.h>


typedef struct HeapMemStaticsInfo  {
    uint32_t totalSize;
    uint32_t usedSize;
    uint32_t freeSize;
    uint32_t maxusedsize;
    uint32_t allocCount;
    uint32_t freeCount;
    uint32_t failureCount;
}HeapMemStaticsInfo;


int MemHeapCreate(void* address, int pool_sz);
void* MemHeapMalloc(int sz,const char* callfuction);
int MemHeapFree( void* p,const char* callfuction );
void MemHeapStatistics(HeapMemStaticsInfo* pinfo) ;





#ifdef __cplusplus
}
#endif
