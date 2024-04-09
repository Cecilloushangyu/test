#include <stdint.h>
#include <stdlib.h>
#include "mempool.h"
#include "memcfg.h"


osMemPoolCfg g_MemPoolCfg[] =
{
	{POOL_SIZE08,128},
	{POOL_SIZE16,128},
	{POOL_SIZE32,128},
	{POOL_SIZE64,64},
	{POOL_SIZE128,64},
	{POOL_SIZE256,64},
	{POOL_SIZE512,16},
	{POOL_SIZE1024,16},
	{POOL_SIZE2048,16},
	{POOL_SIZE4096,8},
	{POOL_SIZE8192,4}
};

uint32_t MemPoolGetPoolTypeCount()
{

	return (sizeof(g_MemPoolCfg)/sizeof(osMemPoolCfg));

}

uint32_t MemPoolGetMaxPoolSize()
{

	int pool_count = MemPoolGetPoolTypeCount();
	uint32_t size  ;

	size = 0;
    for(int i = 0; i < pool_count; i++){
		if(g_MemPoolCfg[i].pool_size>size)
		size = g_MemPoolCfg[i].pool_size;
    }

	return size;
}


osMemPoolCfg* MemPoolGetCfg(int index)
{
	return &g_MemPoolCfg[index];
}


uint32_t MemPoolGetTotalSize()
{
	int pool_count = MemPoolGetPoolTypeCount();
	uint32_t size  ;

	size = 0;
    for(int i = 0; i < pool_count; i++){
		size += (g_MemPoolCfg[i].pool_num*g_MemPoolCfg[i].pool_size);
    }
	return size;
}



