
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdbool.h>
#include <math.h>
#include "memblock.h"
#include "memcfg.h"







#define OS_ENTER_CRITICAL()   	
#define OS_EXIT_CRITICAL()   

MemPoolBlock *g_OSMemFreeList;            /* Pointer to free list of memory partitions       */
MemPoolBlock  g_OSMemTbl[OS_MAX_MEM_PART];/* Storage for memory partition manager */


uint8_t  g_OSMemBlockIsInited =0;   


/*
*********************************************************************************************************
*                                        CREATE A MEMORY PARTITION
*
* Description : Create a fixed-sized memory partition that will be managed by uC/OS-II.
*
* Arguments   : addr     is the starting address of the memory partition
*
*               nblks    is the number of memory blocks to create from the partition.
*
*               blksize  is the size (in bytes) of each block in the memory partition.
*
*               perr     is a pointer to a variable containing an error message which will be set by
*                        this function to either:
*
*                        OS_ERR_NONE              if the memory partition has been created correctly.
*                        OS_ERR_MEM_INVALID_ADDR  if you are specifying an invalid address for the memory
*                                                 storage of the partition or, the block does not align
*                                                 on a pointer boundary
*                        OS_ERR_MEM_INVALID_PART  no free partitions available
*                        OS_ERR_MEM_INVALID_BLKS  user specified an invalid number of blocks (must be >= 2)
*                        OS_ERR_MEM_INVALID_SIZE  user specified an invalid block size
*                                                   - must be greater than the size of a pointer
*                                                   - must be able to hold an integral number of pointers
* Returns    : != (MemPoolBlock *)0  is the partition was created
*              == (MemPoolBlock *)0  if the partition was not created because of invalid arguments or, no
*                              free partition is available.
*********************************************************************************************************
*/

MemPoolBlock  *MemBlockCreate (void   *addr,
                      INT32U  nblks,
                      INT32U  blksize,
                      INT8U  *perr)
{
    MemPoolBlock    *pmem;
    INT8U     *pblk;
    void     **plink;
    INT32U     loops;
    INT32U     i;


    if (addr == (void *)0) {                          /* Must pass a valid address for the memory part.*/
        *perr = OS_ERR_MEM_INVALID_ADDR;
        return ((MemPoolBlock *)0);
    }
    if (((INT64U)addr & (sizeof(void *) - 1u)) != 0u){  /* Must be pointer size aligned                */
        *perr = OS_ERR_MEM_INVALID_ADDR;
        return ((MemPoolBlock *)0);
    }
    if (nblks < 2u) {                                 /* Must have at least 2 blocks per partition     */
        *perr = OS_ERR_MEM_INVALID_BLKS;
        return ((MemPoolBlock *)0);
    }
    if (blksize < sizeof(void *)) {                   /* Must contain space for at least a pointer     */
        *perr = OS_ERR_MEM_INVALID_SIZE;
        return ((MemPoolBlock *)0);
    }

    OS_ENTER_CRITICAL();
    pmem = g_OSMemFreeList;                             /* Get next free memory partition                */
    if (g_OSMemFreeList != (MemPoolBlock *)0) {               /* See if pool of free partitions was empty      */
        g_OSMemFreeList = (MemPoolBlock *)g_OSMemFreeList->OSMemFreeList;
    }
    OS_EXIT_CRITICAL();
    if (pmem == (MemPoolBlock *)0) {                        /* See if we have a memory partition             */
        *perr = OS_ERR_MEM_INVALID_PART;
        return ((MemPoolBlock *)0);
    }
    plink = (void **)addr;                            /* Create linked list of free memory blocks      */
    pblk  = (INT8U *)addr;
    loops  = nblks - 1u;
    for (i = 0u; i < loops; i++) {
        pblk +=  blksize;                             /* Point to the FOLLOWING block                  */
       *plink = (void  *)pblk;                        /* Save pointer to NEXT block in CURRENT block   */
        plink = (void **)pblk;                        /* Position to  NEXT      block                  */
    }
    *plink              = (void *)0;                  /* Last memory block points to NULL              */
    pmem->OSMemAddr     = addr;                       /* Store start address of memory partition       */
    pmem->OSMemFreeList = addr;                       /* Initialize pointer to pool of free blocks     */
    pmem->OSMemNFree    = nblks;                      /* Store number of free blocks in MCB            */
    pmem->OSMemNBlks    = nblks;
    pmem->OSMemBlkSize  = blksize;                    /* Store block size of each memory blocks        */
    pmem->OSMemEndAddr  = addr+nblks*blksize;  
    pmem->OSMallocCount  =  0; 
    pmem->OSFreeCount  =  0; 
    *perr               = OS_ERR_NONE;
    return (pmem);
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                          GET A MEMORY BLOCK
*
* Description : Get a memory block from a partition
*
* Arguments   : pmem    is a pointer to the memory partition control block
*
*               perr    is a pointer to a variable containing an error message which will be set by this
*                       function to either:
*
*                       OS_ERR_NONE             if the memory partition has been created correctly.
*                       OS_ERR_MEM_NO_FREE_BLKS if there are no more free memory blocks to allocate to caller
*                       OS_ERR_MEM_INVALID_PMEM if you passed a NULL pointer for 'pmem'
*
* Returns     : A pointer to a memory block if no error is detected
*               A pointer to NULL if an error is detected
*********************************************************************************************************
*/

void  *MemBlockGet (MemPoolBlock  *pmem,
                 INT8U   *perr)
{
    void      *pblk;

     INT32U  OSMemNUsed;  

    if (pmem == (MemPoolBlock *)0) {                        /* Must point to a valid memory partition        */
        *perr = OS_ERR_MEM_INVALID_PMEM;
        return ((void *)0);
    }

    OS_ENTER_CRITICAL();
    if (pmem->OSMemNFree > 0u) {                      /* See if there are any free memory blocks       */
        pblk                = pmem->OSMemFreeList;    /* Yes, point to next free memory block          */
        pmem->OSMemFreeList = *(void **)pblk;         /*      Adjust pointer to new free list          */
        pmem->OSMemNFree--;                           /*      One less memory block in this partition  */
        OS_EXIT_CRITICAL();
        *perr = OS_ERR_NONE;                          /*      No error                                 */
        OSMemNUsed =  (pmem->OSMemNBlks-pmem->OSMemNFree);
        if(OSMemNUsed> pmem->OSMaxUsedNBlks)
        pmem->OSMaxUsedNBlks = OSMemNUsed;
        pmem->OSMallocCount++; 
        return (pblk);                                /*      Return memory block to caller            */
    }
    OS_EXIT_CRITICAL();
    *perr = OS_ERR_MEM_NO_FREE_BLKS;                  /* No,  Notify caller of empty memory partition  */

    pmem->OSMallocFailCount++; 

    return ((void *)0);                               /*      Return NULL pointer to caller            */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                   GET THE NAME OF A MEMORY PARTITION
*
* Description: This function is used to obtain the name assigned to a memory partition.
*
* Arguments  : pmem      is a pointer to the memory partition
*
*              pname     is a pointer to a pointer to an ASCII string that will receive the name of the memory partition.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        OS_ERR_NONE                if the name was copied to 'pname'
*                        OS_ERR_MEM_INVALID_PMEM    if you passed a NULL pointer for 'pmem'
*                        OS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        OS_ERR_NAME_GET_ISR        You called this function from an ISR
*
* Returns    : The length of the string or 0 if 'pmem' is a NULL pointer.
*********************************************************************************************************
*/


INT8U  MemBlockNameGet (MemPoolBlock   *pmem,
                     INT8U   **pname,
                     INT8U    *perr)
{
    INT8U      len;





    if (pmem == (MemPoolBlock *)0) {                   /* Is 'pmem' a NULL pointer?                          */
        *perr = OS_ERR_MEM_INVALID_PMEM;
        return (0u);
    }
    if (pname == (INT8U **)0) {                  /* Is 'pname' a NULL pointer?                         */
        *perr = OS_ERR_PNAME_NULL;
        return (0u);
    }
#if 0
    if (ISR_IsIrqProcessing() > 0u) {                     /* See if trying to call from an ISR                  */
        *perr = OS_ERR_NAME_GET_ISR;
        return (0u);
    }
#endif
    OS_ENTER_CRITICAL();
    *pname = pmem->OSMemName;
    len    = strlen((char*)*pname);
    OS_EXIT_CRITICAL();
    *perr  = OS_ERR_NONE;
    return (len);
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                 ASSIGN A NAME TO A MEMORY PARTITION
*
* Description: This function assigns a name to a memory partition.
*
* Arguments  : pmem      is a pointer to the memory partition
*
*              pname     is a pointer to an ASCII string that contains the name of the memory partition.
*
*              perr      is a pointer to an error code that can contain one of the following values:
*
*                        OS_ERR_NONE                if the name was copied to 'pname'
*                        OS_ERR_MEM_INVALID_PMEM    if you passed a NULL pointer for 'pmem'
*                        OS_ERR_PNAME_NULL          You passed a NULL pointer for 'pname'
*                        OS_ERR_MEM_NAME_TOO_LONG   if the name doesn't fit in the storage area
*                        OS_ERR_NAME_SET_ISR        if you called this function from an ISR
*
* Returns    : None
*********************************************************************************************************
*/


void  MemBlockNameSet (MemPoolBlock  *pmem,
                    INT8U   *pname,
                    INT8U   *perr)
{


    if (pmem == (MemPoolBlock *)0) {                   /* Is 'pmem' a NULL pointer?                          */
        *perr = OS_ERR_MEM_INVALID_PMEM;
        return;
    }
    if (pname == (INT8U *)0) {                   /* Is 'pname' a NULL pointer?                         */
        *perr = OS_ERR_PNAME_NULL;
        return;
    }


    OS_ENTER_CRITICAL();
    pmem->OSMemName = pname;
    OS_EXIT_CRITICAL();
    *perr           = OS_ERR_NONE;
}


/*$PAGE*/
/*
*********************************************************************************************************
*                                         RELEASE A MEMORY BLOCK
*
* Description : Returns a memory block to a partition
*
* Arguments   : pmem    is a pointer to the memory partition control block
*
*               pblk    is a pointer to the memory block being released.
*
* Returns     : OS_ERR_NONE              if the memory block was inserted into the partition
*               OS_ERR_MEM_FULL          if you are returning a memory block to an already FULL memory
*                                        partition (You freed more blocks than you allocated!)
*               OS_ERR_MEM_INVALID_PMEM  if you passed a NULL pointer for 'pmem'
*               OS_ERR_MEM_INVALID_PBLK  if you passed a NULL pointer for the block to release.
*********************************************************************************************************
*/

INT8U  MemBlockPut (MemPoolBlock  *pmem,
                 void    *pblk)
{



    if (pmem == (MemPoolBlock *)0) {                   /* Must point to a valid memory partition             */
        return (OS_ERR_MEM_INVALID_PMEM);
    }
    if (pblk == (void *)0) {                     /* Must release a valid block                         */
        return (OS_ERR_MEM_INVALID_PBLK);
    }

    OS_ENTER_CRITICAL();
    if (pmem->OSMemNFree >= pmem->OSMemNBlks) {  /* Make sure all blocks not already returned          */
        OS_EXIT_CRITICAL();
        return (OS_ERR_MEM_FULL);
    }
    *(void **)pblk      = pmem->OSMemFreeList;   /* Insert released block into free block list         */
    pmem->OSMemFreeList = pblk;
    pmem->OSMemNFree++;                          /* One more memory block in this partition            */
    OS_EXIT_CRITICAL();
    pmem->OSFreeCount++; 
    return (OS_ERR_NONE);                        /* Notify caller that memory block was released       */
}
/*$PAGE*/
/*
*********************************************************************************************************
*                                          QUERY MEMORY PARTITION
*
* Description : This function is used to determine the number of free memory blocks and the number of
*               used memory blocks from a memory partition.
*
* Arguments   : pmem        is a pointer to the memory partition control block
*
*               p_mem_data  is a pointer to a structure that will contain information about the memory
*                           partition.
*
* Returns     : OS_ERR_NONE               if no errors were found.
*               OS_ERR_MEM_INVALID_PMEM   if you passed a NULL pointer for 'pmem'
*               OS_ERR_MEM_INVALID_PDATA  if you passed a NULL pointer to the data recipient.
*********************************************************************************************************
*/

#if 0
INT8U  MemQuery (MemPoolBlock       *pmem,
                   OS_MEM_DATA  *p_mem_data)
{


    if (pmem == (MemPoolBlock *)0) {                   /* Must point to a valid memory partition             */
        return (OS_ERR_MEM_INVALID_PMEM);
    }
    if (p_mem_data == (OS_MEM_DATA *)0) {        /* Must release a valid storage area for the data     */
        return (OS_ERR_MEM_INVALID_PDATA);
    }
    OS_ENTER_CRITICAL();
    p_mem_data->OSAddr     = pmem->OSMemAddr;
    p_mem_data->OSFreeList = pmem->OSMemFreeList;
    p_mem_data->OSBlkSize  = pmem->OSMemBlkSize;
    p_mem_data->OSNBlks    = pmem->OSMemNBlks;
    p_mem_data->OSNFree    = pmem->OSMemNFree;
    OS_EXIT_CRITICAL();
    p_mem_data->OSNUsed    = p_mem_data->OSNBlks - p_mem_data->OSNFree;
    return (OS_ERR_NONE);
}
#endif                                        /* OS_MEM_QUERY_EN                                    */
/*$PAGE*/
/*
*********************************************************************************************************
*                                    INITIALIZE MEMORY PARTITION MANAGER
*
* Description : This function is called by uC/OS-II to initialize the memory partition manager.  Your
*               application MUST NOT call this function.
*
* Arguments   : none
*
* Returns     : none
*
* Note(s)    : This function is INTERNAL to uC/OS-II and your application should not call it.
*********************************************************************************************************
*/

void  MemBlockInit (void)
{

    MemPoolBlock  *pmem;
    INT16U   i;
    if(g_OSMemBlockIsInited !=0) return ;

    memset((INT8U *)&g_OSMemTbl[0], 0, sizeof(g_OSMemTbl));   /* Clear the memory partition table          */
    for (i = 0u; i < (OS_MAX_MEM_PART - 1u); i++) {       /* Init. list of free memory partitions      */
        pmem                = &g_OSMemTbl[i];               /* Point to memory control block (MCB)       */
        pmem->OSMemFreeList = (void *)&g_OSMemTbl[i + 1u];  /* Chain list of free partitions             */
        pmem->OSMemName  = (INT8U *)(void *)"?";

    }
    pmem                = &g_OSMemTbl[i];
    pmem->OSMemFreeList = (void *)0;                      /* Initialize last node                      */

    pmem->OSMemName = (INT8U *)(void *)"?";

    g_OSMemFreeList   = &g_OSMemTbl[0];                       /* Point to beginning of free list           */

    g_OSMemBlockIsInited  =1 ;
}


int CheckMemPoolBlockByPtr(MemPoolBlock* pblock,void *memory_ptr )
{

    if(pblock ==NULL )	return 0;
    if((memory_ptr >= pblock->OSMemAddr )
       &&(memory_ptr < pblock->OSMemEndAddr )){
        return 1;
    }

	return 0;
}


// 获取 内存池  剩余偏移大小
// poolblock_start    memory_ptr                      poolblock_end
//                      |                                 |  
//                      |     offset_blockend_out         |  
int GetPoolMemDisSize(MemPoolBlock* pInfo, uint8_t *memory_ptr, uint8_t ** poolblock_start ,uint8_t ** poolblock_end){

    // The pointer is already NULL.
     //   osPrintf("osFreeMem memory_ptr  %x  \r\n",memory_ptr);


	uint16_t i;
    int32_t offset;
    uint8_t*  mempoolstart = pInfo->OSMemAddr;
    uint8_t*  memstart;
    uint8_t*  memend;
	for(i = 0; i < pInfo->OSMemNBlks; i++)
	{
	     memstart =(mempoolstart+(pInfo->OSMemBlkSize*i));
         memend = (memstart + pInfo->OSMemBlkSize) ;
		if(memory_ptr >= memstart && memory_ptr < memend){
            offset = (int32_t)(memend - memory_ptr);
            (*poolblock_start) = memstart;
            (*poolblock_end) =  memend;
            return offset;
          }

	}
	//没有找到 返回-1
    return -1;
   // return index_pool(memory_ptr,pInfo->pool_addr,pInfo->pool_num,pInfo->pool_size,&poolblock_start,&poolblock_end);
  
}


