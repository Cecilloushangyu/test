
#ifndef RTK_DEMO_H_
#define RTK_DEMO_H_

#include "ks_datatypes.h"

extern PvtResult g_pvt_result;

void SavePvtResult(const U8 *buffer, U32 msg_len);
void TriggerResultPrinting();
void RtkInit();


#endif //RTK_DEMO_H_
