

#pragma  once


#include <stdint.h>


#define	POOL_SIZE08			8
#define	POOL_SIZE16			16
#define	POOL_SIZE32     	32
#define	POOL_SIZE64     	64
#define	POOL_SIZE128     	128
#define	POOL_SIZE256      	256
#define	POOL_SIZE512     	512
#define	POOL_SIZE1024    	1024
#define	POOL_SIZE2048    	2048
#define	POOL_SIZE4096	 	4096
#define	POOL_SIZE8192	 	8192


typedef struct osMemPoolCfg{
	uint32_t pool_size;	// size of the pool, in byte
	uint32_t pool_num;	// number of the pool
} osMemPoolCfg;


uint32_t MemPoolGetTotalSize();
osMemPoolCfg* MemPoolGetCfg(int index);
uint32_t MemPoolGetPoolTypeCount();
uint32_t MemPoolGetMaxPoolSize();



