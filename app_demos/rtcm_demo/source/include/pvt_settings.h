#pragma once
#include "ks_datatypes.h"

#ifdef EMBEDDED
#define _ATTRIBUTE_ALIGN_8_ __attribute__((aligned(8)))
#else
#define _ATTRIBUTE_ALIGN_8_
#endif
