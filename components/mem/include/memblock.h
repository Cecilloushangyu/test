#ifndef __CMSIS_MEMBLOCK_H__
#define __CMSIS_MEMBLOCK_H__


#define OS_ERR_MEM_INVALID_PART        90u
#define OS_ERR_MEM_INVALID_BLKS        91u
#define OS_ERR_MEM_INVALID_SIZE        92u
#define OS_ERR_MEM_NO_FREE_BLKS        93u
#define OS_ERR_MEM_FULL                94u
#define OS_ERR_MEM_INVALID_PBLK        95u
#define OS_ERR_MEM_INVALID_PMEM        96u
#define OS_ERR_MEM_INVALID_PDATA       97u
#define OS_ERR_MEM_INVALID_ADDR        98u
#define OS_ERR_MEM_NAME_TOO_LONG       99u

#define OS_ERR_NONE                     0u

#define OS_ERR_TIMEOUT                 10u
#define OS_ERR_EVENT_NAME_TOO_LONG     11u
#define OS_ERR_PNAME_NULL              12u
#define OS_ERR_PEND_LOCKED             13u
#define OS_ERR_PEND_ABORT              14u
#define OS_ERR_DEL_ISR                 15u
#define OS_ERR_CREATE_ISR              16u
#define OS_ERR_NAME_GET_ISR            17u
#define OS_ERR_NAME_SET_ISR            18u
#define OS_ERR_ILLEGAL_CREATE_RUN_TIME 19u

#define OS_MAX_MEM_PART           30u   /* Max. number of memory partitions                             */


typedef unsigned char  INT8U;   /* 无符号8位整型变量      */
typedef signed   char  INT8S;   /* 有符号8位整型变量      */

typedef unsigned short INT16U;  /* 无符号16位整型变量     */
typedef signed   short INT16S;  /* 有符号16位整型变量     */


typedef unsigned int   INT32U;  /* 无符号32位整型变量     */
typedef signed   int   INT32S;  /* 有符号32位整型变量     */


typedef unsigned long   INT64U;  /* 无符号64位整型变量     */
typedef signed   long   INT64S;  /* 有符号63位整型变量     */


typedef struct MemPoolBlock {                   /* MEMORY CONTROL BLOCK                                      */
	INT8U  *OSMemName;                    /* Memory partition name */
    void   *OSMemAddr;                    /* Poinye'ster to beginning of memory partition                  */
    void   *OSMemFreeList;                /* Pointer to list of free memory blocks                     */
    void   *OSMemEndAddr;                    /* Pointer to end of memory partition  */
    INT32U  OSMemBlkSize;                 /* Size (in bytes) of each block of memory                   */
    INT32U  OSMemNBlks;                   /* Total number of blocks in this partition                  */
    INT32U  OSMemNFree;                   /* Number of memory blocks remaining in this partition       */
    INT32U  OSMaxUsedNBlks;
    INT32U  OSMallocCount;
    INT32U  OSFreeCount;
    INT32U  OSMallocFailCount;
} MemPoolBlock;

void  MemBlockInit (void);
MemPoolBlock  *MemBlockCreate (void   *addr,
                      INT32U  nblks,
                      INT32U  blksize,
                      INT8U  *perr);
void  *MemBlockGet (MemPoolBlock  *pmem,
                 INT8U   *perr);

INT8U	MemBlockPut (MemPoolBlock  *pmem,
			  void	  *pblk);

#endif
