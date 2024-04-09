#ifndef INTERFACE_INCLUDE_AP_CAN_H_
#define INTERFACE_INCLUDE_AP_CAN_H_

#include "ks_datatypes.h"

#define CAN_NUM 2		//!< 最多支持2个CAN口，分别为串口2和串口3复用
#define CAN_START 0

typedef struct
{
	U32 id;
	U8 state;	//!< bit7 FF, bit 6 RTR, bit 3~0 DLC
	U8 data[8];
} CanFrameData, *P_CanFrameData;

typedef S32 (*CanEntry)(U32, CanFrameData);

void ks_driver_can_init(U32 can_id);
void ks_driver_can_register_callback(U32 can_id, CanEntry p_call_back);
void ks_driver_can_deregister_callback(U32 can_id);
void ks_driver_can_send_frame(U32 can_id, U32 frame_format, U32 remote, U32 frame_id, const U8* p_data, S32 len);
void ks_driver_can_set_id_filter(U32 can_id, U32 code_bit, U32 mask_bit, BOOL is_dual_filter);
void ks_driver_can_set_baud(U32 can_id, U32 baud_rate);

void ks_driver_can_poll_send_frame(U32 id, U32 frame_format, U32 remote, U32 frame_id, const U8* p_data, S32 len, BOOL b_single_shot);

void ks_driver_can_reset(S32 id);
void ks_driver_can_dump_info(S32 id);

#endif
