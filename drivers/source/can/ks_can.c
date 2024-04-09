#include "ks_can.h"
#include "can_controller.h"

void ks_driver_can_init(U32 can_id)
{
	_init_can(can_id);

}

void ks_driver_can_register_callback(U32 can_id, CanEntry p_call_back)
{
	if (can_id < CAN_NUM)
		_can_register_callback(can_id, p_call_back);
}

void ks_driver_can_deregister_callback(U32 can_id)
{
	if (can_id < CAN_NUM)
		_can_deregister_callback(can_id);
}

void ks_driver_can_send_frame(U32 can_id, U32 frame_format, U32 remote, U32 frame_id, const U8* p_data, S32 len)
{
	if (can_id < CAN_NUM)
		_can_send_frame(can_id, frame_format, remote, frame_id, p_data, len);
}

void ks_driver_can_poll_send_frame(U32 can_id, U32 frame_format, U32 remote, U32 frame_id, const U8* p_data, S32 len, BOOL b_single_shot)
{
	if (can_id < CAN_NUM)
		_can_poll_send_frame(can_id, frame_format, remote, frame_id, p_data, len, b_single_shot);
}

void ks_driver_can_set_id_filter(U32 can_id, U32 code_bit, U32 mask_bit, BOOL is_dual_filter)
{
	if (can_id < CAN_NUM)
		_can_set_id_filter(can_id, code_bit, mask_bit, is_dual_filter);
}

void ks_driver_can_set_baud(U32 can_id, U32 baud_rate)
{
	_can_set_baud(can_id, baud_rate);
}

void ks_driver_can_reset(S32 id)
{
	_can_reset(id);
}

void ks_driver_can_dump_info(S32 id)
{
	return _can_dump_info(id);
}
