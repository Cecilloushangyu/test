#ifndef _CAN_CONTROLLER_H_
#define _CAN_CONTROLLER_H_

#include <ks_can.h>
#include "mg_mcan2_define.h"

void _init_can(U32 id);
void _can_register_callback(U32 id, CanEntry p_callback);
void _can_deregister_callback(U32 id);
void _can_send_frame(U32 id, U32 frame_format, U32 remote, U32 frame_id, const U8* p_data, S32 len);
void _can_set_id_filter(U32 id, U32 acr, U32 amr, BOOL is_dual);
void _can_poll_send_frame(U32 id, U32 frame_format, U32 remote, U32 frame_id, const U8* p_data, S32 len, BOOL b_single_shot);
void _can_set_baud(U32 id, U32 baudrate);

void _can_dump_info(S32 id);
void _can_reset(S32 id);
#endif
