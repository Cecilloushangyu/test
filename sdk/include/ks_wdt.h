#pragma once

#include "ks_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

void ks_wdt_enable(BOOL b_enable, U32 period);
void ks_wdt_feed_wdt();


#ifdef __cplusplus
}
#endif
