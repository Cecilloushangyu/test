
#ifndef _MEM_H_
#define _MEM_H_

#include <stdarg.h>
#include <stddef.h>
#include <ks_datatypes.h>

#ifdef __cplusplus
extern "C" {
#endif


void ks_mem_init();

void* ks_mem_pool_malloc(uint32_t size);
S32 ks_mem_pool_free(void *pPtr);

void* ks_mem_heap_malloc(uint32_t size);
S32 ks_mem_heap_free(void *pPtr);




#ifdef __cplusplus
}
#endif


#endif 
