
#ifndef HEADING_DEMO_H_
#define HEADING_DEMO_H_

#include "ks_datatypes.h"

extern PvtResult g_pvt_result;

void SavePvtResult(const U8 *buffer, U32 msg_len);
void TriggerResultPrinting();
void RtkInit();


#endif //HEADING_DEMO_H_
