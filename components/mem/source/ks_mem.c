

#include <stdarg.h>
#include <stddef.h>
#include <string.h>
#include "ks_datatypes.h"
#include "ks_shell.h"
#include "mempool.h"
#include "memheap.h"
#include "mem_demo.h"
#include "memcfg.h"
#include "ks_mem.h"
#include "ks_section.h"
#include "ks_printf.h"
#include "ks_os.h"
#include "ks_exception.h"


#define __FUNCTION__ " "

static unsigned char mempool_region[0x40000] __attribute__ ((aligned(8)));
static unsigned char memheap_region[0x40000] __attribute__ ((aligned(8)));
static int isinited;


S32 mempool(cmd_proc_t* ctx,S32 argc,  char **argv)
{
	ks_shell_printf(ctx->uart_id,"\r\n**************************************************\r\n");
	ks_shell_printf(ctx->uart_id, "mempool\r\n");

	MemPoolStaticsInfo  info ;
	MemPoolCtx*  pctx =  MemPoolCtxGet();
	for(S32 i = 0; i < pctx->pool_cnt; i++)
	{	
		MemPoolBlock *	pblock = pctx->pool_block[i];
		MemPoolStatistics(pblock,&info);
		ks_shell_printf(ctx->uart_id,"BlockSize %5d  totalCount %3d  Used %3d  UnUsed %3d  maxusedCount %3d  MallocCount %6d  FreeCount %6d  FailureCount %6d \r\n",pblock->OSMemBlkSize,info.totalCount,info.usedCount,info.unusedCount,info.maxusedCount,info.mallocCount, info.freeCount, info.failureCount);

	}
	ks_shell_printf(ctx->uart_id,"\r\n***************************************************\r\n");
	return 0;
}


S32 memheap(cmd_proc_t* ctx,S32 argc,  char **argv )
{
    ks_shell_printf(ctx->uart_id,"\r\n**************************************************\r\n");
    ks_shell_printf(ctx->uart_id, "memheap\r\n");
    HeapMemStaticsInfo  info ;

    MemHeapStatistics(&info);
    ks_shell_printf(ctx->uart_id,"totalSize %d(%dK)  usedSize  %d(%dK)   freeSize %d(%dK) maxusedsize %d(%dK) allocCount %d  freeCount %d  failureCount %d \r\n",
    info.totalSize,info.totalSize/1024,
    info.usedSize,info.usedSize/1024,
    info.freeSize,  info.freeSize/1024,
    info.maxusedsize,  info.maxusedsize/1024,
    info.allocCount,
    info.freeCount,
    info.failureCount);

    ks_shell_printf(ctx->uart_id,"\r\n***************************************************\r\n");

	return 0;

}

S32  mempooltest(  cmd_proc_t* ctx,S32 argc,  char **argv){
	void * pmem; 
	S32 maxsize = MemPoolGetMaxPoolSize();
	for(S32 i =1 ; i<= maxsize;i++){

		pmem=  ks_mem_pool_malloc(i);
		if(pmem!=NULL) {
			memset(pmem,0,i);
			ks_shell_printf(ctx->uart_id,"ks_mem_pool_malloc()   size   %d  \r\n",   i);
			ks_mem_pool_free(pmem);
			ks_shell_printf(ctx->uart_id,"ks_mem_pool_free()   size  %d \r\n",   i);
			ks_os_thread_sleep_msec(10);
		}else{
			ks_exception_assert(0);
		}
	}

	return 0;
}


S32  memheaptest(  cmd_proc_t* ctx,S32 argc,  char **argv){
	void * pmem; 
	for(S32 i =1 ; i<= 1024*4;i++){
		pmem=  ks_mem_heap_malloc(i);
		if(pmem!=NULL) {
			memset(pmem,0,i);
			ks_shell_printf(ctx->uart_id,"ks_mem_heap_malloc()   size	 %d  \r\n",   i);
			ks_mem_heap_free(pmem);
			ks_shell_printf(ctx->uart_id,"ks_mem_heap_free()	size  %d \r\n",   i);
			ks_os_thread_sleep_msec(10);
		}else{
			ks_exception_assert(0);
		}
	}
	return 0;
}

static cmd_proc_t mem_cmds[] = {
	{.cmd = "mempool", .fn = mempool, .help = "mempool "},
	{.cmd = "memheap", .fn = memheap, .help = "memheap "},
	{.cmd = "mempooltest", .fn = mempooltest, .help = "mempooltest "},
	{.cmd = "memheaptest", .fn = memheaptest, .help = "memheaptest "},
	//{.cmd = "msgtest", .fn = msgtest, .help = "msgtest <client> <enable/disable>"},
};



void ks_mem_init(){

	S32 memheap_size,heap_size,mempool_size,pool_size;
	if(isinited == 1) return;

	void* mempool_start = (void*)mempool_region;
	mempool_size = sizeof(mempool_region);
	void* mempool_end = mempool_start + mempool_size;
	memset(mempool_start,0,mempool_size);
	pool_size = MemPoolCreate(mempool_start,mempool_size);
	//kprintf("mempool_start %x  mempool_end %x mempool_size %dk pool_size %dk  \r\n",mempool_start,mempool_end,mempool_size/1024,pool_size/1024);



	void*  memheap_start =(void*)memheap_region;
	memheap_size = sizeof(memheap_region);
	void*  memheap_end = memheap_region + memheap_size;
	memset(memheap_start,0,memheap_size);
	heap_size = MemHeapCreate(memheap_start,memheap_size);
	//kprintf("memheap_start %x  memheap_end %x memheap_size %dk heap_size %dk  \r\n",memheap_start,memheap_end,memheap_size/1024,heap_size/1024);
	isinited = 1;

	ks_shell_add_cmds(mem_cmds, sizeof(mem_cmds) / sizeof(cmd_proc_t));


}

void* ks_mem_pool_malloc(uint32_t size){
    void* pMem;
  	ks_os_irq_mask_all();
    pMem = MemPoolMalloc(size,__FUNCTION__);
    ks_os_irq_unmask_all();
    return pMem;


}

S32 ks_mem_pool_free(void *pPtr){

	S32 ret;

  	ks_os_irq_mask_all();
	ret = MemPoolFree(pPtr,__FUNCTION__);
	ks_os_irq_unmask_all();
	return ret;


}

void* ks_mem_heap_malloc(uint32_t size){
	void* pMem;
    ks_os_irq_mask_all();
	pMem = MemHeapMalloc(size,__FUNCTION__);
	ks_os_irq_unmask_all();

	return pMem;
}

S32  ks_mem_heap_free(void *pPtr){


	S32 ret;
  	ks_os_irq_mask_all();
	ret = MemHeapFree(pPtr,__FUNCTION__);
	ks_os_irq_unmask_all();

	return ret;


}




