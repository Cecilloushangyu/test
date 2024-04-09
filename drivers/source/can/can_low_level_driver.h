#ifndef _CAN_LOW_LEVEL_DRIVER_H_
#define _CAN_LOW_LEVEL_DRIVER_H_

#include "mg_mcan2_define.h"
#include "ks_driver.h"

typedef struct
{
	U32 can_addr;
	U32 can_baud;
} CANHandle, *P_CANHandle;

void _can_low_level_init(S32 id, CANHandle* p_can_handle);
void _can_low_level_set_baud_rate(CANHandle* p_can_handle, U32 baud);
void _can_low_level_read_rx_fifo(CANHandle* p_can_handle, U8 dest_buffer[13]);
U32 _can_low_level_read_interrupt(CANHandle* p_can_handle);
void _can_low_level_config_interrupt(CANHandle* p_can_handle, U8 enable_int);
void _can_low_level_set_acceptance_filter(CANHandle* p_can_handle, U8 acr[4], U8 amr[4], BOOL b_is_dual_filter);
void _can_low_level_send_frame(CANHandle* p_can_handle, S32 frame_format, S32 remote, U32 frame_id, const U8* p_data, S32 len, BOOL b_single_shot);
#endif
