#pragma once


#define SECTION(x)					__attribute__((section(x)))
#define UNUSED						__attribute__((unused))
#define USED						__attribute__((used))
#define ALIGN(n)					__attribute__((aligned(n)))
#define WEAK 						__attribute__((weak))
#define PACKED 						__attribute__((packed))
#define INLINE 						 static __inline



