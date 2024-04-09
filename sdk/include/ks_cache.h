#pragma once
#include <stdint.h>

void ks_cpu_icache_invalidate(void *addr, int size);
void ks_cpu_dcache_invalidate(void *addr, int size);
void ks_cpu_dcache_clean(void *addr, int size);
void ks_cpu_dcache_clean_and_invalidate(void *addr, int size);


