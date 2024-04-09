
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include "mempool.h"
#include "memcfg.h"
#include "memblock.h"
#include "ks_shell.h"
#include "ks_os.h"
#include "ks_printf.h"
#include "ks_exception.h"


MemPoolCtx g_MemPoolCtx;


int  MemPoolCreate(void * ptr, uint32_t buffer_size)
{     
	uint32_t i;

    MemBlockInit();

	osMemPoolCfg* pMemPoolCfg;
	uint32_t pool_count ;
    INT8U err;
    osMemPoolCfg* pInfo ;

	uint8_t* poolblock_addr;	
	uint32_t pool_offset ;


	uint32_t  size  = MemPoolGetTotalSize();

   	if(size > buffer_size){
		kprintf("MemPoolGetTotalSize()  > buffer_size   %d	%d \r\n",size,buffer_size );
		ks_exception_assert(0);
	}

	pool_count = MemPoolGetPoolTypeCount();
	//pMemPoolCfg = MemPoolGetCfg();
    g_MemPoolCtx.pool_cnt = pool_count;

   	if(pool_count > sizeof(g_MemPoolCtx.pool_block)/sizeof(void*)){
		kprintf("sizeof(g_MemPoolCtx.pool_block) %d	%d\r\n", sizeof(g_MemPoolCtx.pool_block)/sizeof(void*),pool_count );
		ks_exception_assert(0);
	}

	pool_offset = 0;
	poolblock_addr=ptr;
	
    for( i= 0; i < pool_count; i++)
    {
        //pInfo = pMemPoolCfg+i;
		pInfo = MemPoolGetCfg(i);
        g_MemPoolCtx.pool_block[i] = MemBlockCreate(poolblock_addr+pool_offset,pInfo->pool_num,pInfo->pool_size,&err);
		if(err!=OS_ERR_NONE){
			kprintf("MemBlockCreate()  err %d  i %d  \r\n",err , i);
			ks_exception_assert(0);
			break;
		}		
		pool_offset += (pInfo->pool_num*pInfo->pool_size);
	}

    return size;
}


//二分法快速找到匹配的内存块
MemPoolBlock* MemPoolGetBlockBySize(uint32_t size)
{
	 
	int low = 0;
	int high = g_MemPoolCtx.pool_cnt - 1;
	while (low <= high) {
		 int mid = low + ((high - low) >> 1);
		 if (g_MemPoolCtx.pool_block[mid]->OSMemBlkSize >= size) {
			 // 判断当前是第一个元素或者前一个元素小于等于给定值，则返回下标，如果前一个元素大于给定的值，则继续往前查找。
			 if ((mid == 0) || g_MemPoolCtx.pool_block[mid - 1]->OSMemBlkSize  < size) return g_MemPoolCtx.pool_block[mid];
			 else high = mid - 1;
		 } else {
			 low = mid + 1;
		 }
	}

	return NULL;
}



MemPoolBlock* MemPoolGetBlockByPtr( void *memory_ptr) 
{
	int low = 0;
	int high = g_MemPoolCtx.pool_cnt - 1 ;

	while (low <= high) {
	    // 找出中间下标
	    int mid = low + ((high - low) >> 1);
		
	    if (g_MemPoolCtx.pool_block[mid]->OSMemAddr  > memory_ptr) {
	        high = mid - 1;
	    } else if (g_MemPoolCtx.pool_block[mid]->OSMemEndAddr < memory_ptr) {
	        low = mid + 1;
	    } else {
		    if((memory_ptr >= g_MemPoolCtx.pool_block[mid]->OSMemAddr )
	           &&(memory_ptr < g_MemPoolCtx.pool_block[mid]->OSMemEndAddr )){
	            return g_MemPoolCtx.pool_block[mid];
	        }else{
				return g_MemPoolCtx.pool_block[mid+1];
			}   
	    }
	}

	return NULL;
 }



void * MemPoolMalloc( uint32_t size  ,const char* callfuction ){

    void * ret =NULL;
    INT8U err;
    // Try to find the proper memory pool.
    MemPoolBlock *  pblock = MemPoolGetBlockBySize(size);
    // If no pool is selected, we might allocate this memory in byte pool.
    if(pblock==NULL){
		 ks_shell_printf(0,"\r\n");
		 ks_shell_printf(0,"callfuction: %s  -> MemPoolMalloc　not match size \r\n",callfuction);
         ks_exception_assert( 0) ;
    }

    ret  =  MemBlockGet(pblock,&err);
    if(ret !=NULL && err== 0){
       // osMemset(ret,0,size);
    }else{
#if 1
    	ks_shell_printf(0,"\r\n");
    	ks_shell_printf(0,"callfuction: %s  -> MemPoolMalloc  block is no more free \r\n",callfuction);
     	ks_shell_printf(0,"MemPoolMalloc MemBlockGet ERR!!!  size %d  %d  NULL \r\n",size,err);
		ks_exception_assert(0) ;
#endif
	}

    return ret ;
}

int  MemPoolFree( void *memory_ptr  ,const char* callfuction)
{
    int ret ;
    if (NULL == memory_ptr)
    {
        return -1;
    }

    MemPoolBlock *  pblock=  MemPoolGetBlockByPtr(memory_ptr);
   // osks_shell_printf(0,"osFreeMem sn  %d  \r\n",sn);
    if(pblock==NULL){
		ks_shell_printf(0,"\r\n");
		ks_shell_printf(0,"callfuction: %s  -> MemPoolFree\r\n",callfuction);
		ks_shell_printf(0,"memory has free MemPoolFree ERR!!! \r\n");
        ks_exception_assert( 0) ;
    }

    ret = MemBlockPut(pblock,memory_ptr);
    return ret;
    
}



int MemPoolStatistics(MemPoolBlock* pmem, MemPoolStaticsInfo *info)
{
    if(pmem==NULL) return -1;
    info->totalCount= pmem->OSMemNBlks;
    //info->uwSize = pool_id->item_sz;
    info->unusedCount= pmem->OSMemNFree;
    info->usedCount= pmem->OSMemNBlks - pmem->OSMemNFree;
    info->maxusedCount=pmem->OSMaxUsedNBlks;
    info->mallocCount= pmem->OSMallocCount;
    info->freeCount= pmem->OSFreeCount;
    info->failureCount= pmem->OSMallocFailCount;
    return 0;
}



MemPoolCtx* MemPoolCtxGet()
{
	return &g_MemPoolCtx;
}


                                                                                                                                                                                                                                                                                                                                                  
