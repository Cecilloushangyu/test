#pragma once

#include <stdint.h>
#include <stddef.h>


typedef unsigned long long U64;
typedef long long S64;
typedef unsigned int U32;
typedef signed int S32;
typedef unsigned short U16;
typedef signed short S16;
typedef unsigned char U8;
typedef signed char S8;

typedef float FLOAT;
typedef double DOUBLE;
typedef float F32;
typedef double F64;

typedef int BOOL;


typedef signed long   base_t;   
typedef unsigned long   ubase_t;    


#ifndef FALSE
#define FALSE                                               ((BOOL)0)
#endif

#ifndef TRUE
#define TRUE                                                ((BOOL)1)
#endif

#ifndef NULL
#define NULL                                                ((void *)0)
#endif

typedef volatile U32 REG32;
typedef volatile U16 REG16;
typedef volatile U8 REG8;

#ifdef __64BIT__
typedef S64 PTR;
#else
typedef S32 PTR;
#endif





