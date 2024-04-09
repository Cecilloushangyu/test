#include "can_controller.h"
#include "can_low_level_driver.h"
#include <string.h>
#include "ks_os.h"
#include <stdio.h>
#include "ks_gpio.h"
#include "ks_sysctrl.h"
#include "ks_taskdef.h"

//#define CAN_DEBUG

static CanEntry g_can_callback[CAN_NUM];

// global can
#define CAN_RX_FIFO_SIZE			5
#define CAN_TX_FIFO_SIZE			512

// can receive
#define STACK_SIZE_CAN_ENTRY		768
static OSHandle flag_handle_can_rx;
static OSHandle mutex_handle_can_rx;
static OSHandle thread_handle_can_entry;
static U32 thread_stack_can_entry[STACK_SIZE_CAN_ENTRY];
static void _can_entry_task(void *p_arg);

// can output
#define STACK_SIZE_CAN_OUTPUT		512
static OSHandle flag_handle_can_tx;
static OSHandle mutex_handle_can_tx;
static OSHandle thread_handle_can_output;
static U32 thread_stack_can_output[STACK_SIZE_CAN_OUTPUT];
static void _can_output_task(void *p_arg);

typedef struct tag_can_driver
{
	S32 enable;
	CanFrameData rx_fifo[CAN_RX_FIFO_SIZE];
	CanFrameData tx_fifo[CAN_TX_FIFO_SIZE];
	U8 rx_fifo_read;
	U8 rx_fifo_write;
	U8 rx_fifo_count;
	U16 tx_fifo_read;
	U16 tx_fifo_write;
	U16 tx_fifo_count;
	CANHandle can_handle;
} CANDriver, *P_CANDriver;

static CANDriver g_can[CAN_NUM];

S32 _get_can_irq_vector(S32 id)
{
	switch(id)
	{
	case 0: return IRQ_VEC_CAN0;
	case 1: return IRQ_VEC_CAN1;
	default: break;
	}
	return IRQ_VEC_CAN0;
}


static void _convert_to_frame_data(const U8* temp_buffer, P_CanFrameData p_data)
{
	p_data->state = temp_buffer[0];
	if (temp_buffer[0] & 0x80)
	{
		// EFF
		p_data->id = (((U32)(temp_buffer[1])) << 21) | (((U32)(temp_buffer[2])) << 13) | (((U32)(temp_buffer[3])) << 5) | (((temp_buffer[4]) >> 3) & 0x1F);
		S32 len = temp_buffer[0] & 0xF;
		len = (len > 8) ? 8 : len;
		for (S32 i = 0; i < len; i++)
			p_data->data[i] = temp_buffer[5 + i];
	}
	else
	{
		// SFF
		p_data->id = (((U32)(temp_buffer[1])) << 3) | (((temp_buffer[2]) >> 5) & 0x7);
		S32 len = temp_buffer[0] & 0xF;
		len = (len > 8) ? 8 : len;
		for (S32 i = 0; i < len; i++)
			p_data->data[i] = temp_buffer[3 + i];
	}
}

static void _put_to_rx_buffer(P_CANDriver p_can_driver, P_CanFrameData p_data)
{
	if (p_can_driver->rx_fifo_count == CAN_RX_FIFO_SIZE)
		return;
	ks_os_mutex_enter(mutex_handle_can_rx);
	p_can_driver->rx_fifo[p_can_driver->rx_fifo_write++] = *p_data;
	if (p_can_driver->rx_fifo_write == CAN_RX_FIFO_SIZE)
		p_can_driver->rx_fifo_write = 0;
	p_can_driver->rx_fifo_count++;
	ks_os_mutex_leave(mutex_handle_can_rx);
}

static BOOL _fetch_rx_frame(P_CANDriver p_can_driver, P_CanFrameData p_data)
{
	if (p_can_driver->rx_fifo_count == 0)
		return FALSE;
	ks_os_mutex_enter(mutex_handle_can_rx);
	*p_data = p_can_driver->rx_fifo[p_can_driver->rx_fifo_read++];
	if (p_can_driver->rx_fifo_read == CAN_RX_FIFO_SIZE)
		p_can_driver->rx_fifo_read = 0;
	p_can_driver->rx_fifo_count--;
	ks_os_mutex_leave(mutex_handle_can_rx);
	return TRUE;
}

void _put_to_tx_buffer(P_CANDriver p_can_driver, const CanFrameData* p_data)
{
	ks_os_mutex_enter(mutex_handle_can_tx);
	p_can_driver->tx_fifo[p_can_driver->tx_fifo_write++] = *p_data;
	if (p_can_driver->tx_fifo_write == CAN_TX_FIFO_SIZE)
		p_can_driver->tx_fifo_write = 0;
	p_can_driver->tx_fifo_count++;
	ks_os_mutex_leave(mutex_handle_can_tx);
}

BOOL _fetch_tx_frame(P_CANDriver p_can_driver, CanFrameData* p_data)
{
	if (p_can_driver->tx_fifo_count == 0)
		return FALSE;
	ks_os_mutex_enter(mutex_handle_can_tx);
	*p_data = p_can_driver->tx_fifo[p_can_driver->tx_fifo_read++];
	if (p_can_driver->tx_fifo_read == CAN_TX_FIFO_SIZE)
		p_can_driver->tx_fifo_read = 0;
	p_can_driver->tx_fifo_count--;
	ks_os_mutex_leave(mutex_handle_can_tx);
	return TRUE;
}

BOOL _is_tx_fifo_empty(P_CANDriver p_can_driver)
{
	return p_can_driver->tx_fifo_count ? FALSE : TRUE;
}

void _can_isr_handler( void * arg)
{
	 int id =(int ) arg;
	P_CANDriver p_can_driver = &g_can[id];
	S32 irq_vec_num = _get_can_irq_vector(id + CAN_START);
	U8 temp_buffer[13];
	ks_os_irq_disable(irq_vec_num);
	U32 int_status = _can_low_level_read_interrupt(&p_can_driver->can_handle);
	if (int_status & 0x1)
	{
		_can_low_level_read_rx_fifo(&p_can_driver->can_handle, temp_buffer);
		CanFrameData data;
		_convert_to_frame_data(temp_buffer, &data);
		_put_to_rx_buffer(p_can_driver, &data);
		ks_os_flag_set_bits(flag_handle_can_rx, (1 << id), SET_FLAG_MODE_OR);
#ifdef CAN_DEBUG
		char temp_str[128];
		sprintf(temp_str, "Can Receive, %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X %02X\r\n",
				temp_buffer[0],
				temp_buffer[1],
				temp_buffer[2],
				temp_buffer[3],
				temp_buffer[4],
				temp_buffer[5],
				temp_buffer[6],
				temp_buffer[7],
				temp_buffer[8],
				temp_buffer[9],
				temp_buffer[10],
				temp_buffer[11],
				temp_buffer[12]);
		_uart_send_string(0, temp_str);
#endif
	}
	if (int_status & 0x2)
	{
		if (!_is_tx_fifo_empty(p_can_driver))
			ks_os_flag_set_bits(flag_handle_can_tx, (1 << id), SET_FLAG_MODE_OR);
	}
	if (int_status & 0x4)
	{
#ifdef CAN_DEBUG
		_uart_send_string(0, "Can Error Int Detect\r\n");
#endif
		if (!_is_tx_fifo_empty(p_can_driver))
			ks_os_flag_set_bits(flag_handle_can_tx, (1 << id), SET_FLAG_MODE_OR);
	}
	ks_os_irq_enable(irq_vec_num);
}

void _init_can(U32 id)
{
	static int can_task_created = 0;
	if(id > 1) return ;

	//0. mux
	if(id == 0){
		ks_driver_sysctrl_set_mux_sel(GPIOA_17,MUX_V_FUN2);
		ks_driver_sysctrl_set_mux_sel(GPIOA_18,MUX_V_FUN2);
		ks_driver_sysctrl_set_fuction_sel(CAN0_FUNC_SEL,1);
		ks_driver_sysctrl_set_clock_enable(CAN0_CLK_EN,1);
	}
	else if(id == 1)
	{
		ks_driver_sysctrl_set_mux_sel(GPIOA_19,MUX_V_FUN2);
		ks_driver_sysctrl_set_mux_sel(GPIOA_20,MUX_V_FUN2);
		ks_driver_sysctrl_set_fuction_sel(CAN1_FUNC_SEL,1);
		ks_driver_sysctrl_set_clock_enable(CAN1_CLK_EN,1);
	}

	P_CANDriver p_can_driver = &g_can[id];
	memset(p_can_driver, 0, sizeof(CANDriver));
	p_can_driver->enable = 1;
	_can_low_level_init(id, &p_can_driver->can_handle);
	S32 irq_vec_num = _get_can_irq_vector(id);
	ks_os_irq_create(irq_vec_num, _can_isr_handler, (void*)id);
	ks_os_irq_enable(irq_vec_num);
	ks_os_irq_map_target(irq_vec_num, 1);
	g_can_callback[id] = 0;

	if(can_task_created == 0){

		ks_os_flag_create(&flag_handle_can_tx, "__can_flag_tx");
		ks_os_flag_create(&flag_handle_can_rx, "__can_flag_rx");
		ks_os_mutex_create(&mutex_handle_can_tx, "__can_mutex_tx");
		ks_os_mutex_create(&mutex_handle_can_rx, "__can_mutex_rx");

		ks_os_thread_create(&thread_handle_can_output,	//	thread_handle
				"__ap_platform_can_output_task",	//	thread_name
				_can_output_task,					//	thread_entry
				0,									//	arg
				_THREAD_PRI_CAN_PRINT,				//	priority
				thread_stack_can_output,			//	stack_start
				sizeof(thread_stack_can_output),	//	stack_size
				0,
				1
		);

		ks_os_thread_create(&thread_handle_can_entry,	//	thread_handle
				"__ap_platform_can_entry_task",		//	thread_name
				_can_entry_task,					//	thread_entry
				0,									//	arg
				_THREAD_PRI_CAN_ENTRY,				//	priority
				thread_stack_can_entry,				//	stack_start
				sizeof(thread_stack_can_entry),		//	stack_size
				0,
				1
		);
		can_task_created = 1;
	}
}

void _can_register_callback(U32 id, CanEntry p_callback)
{
	g_can_callback[id] = p_callback;
}

void _can_deregister_callback(U32 id)
{
	g_can_callback[id] = 0;
}

void _can_entry_task(void *p_arg)
{
	ks_os_thread_vfp_enable();
	U32 can_all_rx_flags = (1 << CAN_NUM) - 1;
	while (1)
	{
		U32 flags = ks_os_flag_wait(flag_handle_can_rx, can_all_rx_flags, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
		for (S32 i = 0; i < CAN_NUM; i++)
		{
			if (flags & (1 << i))
			{
				if (g_can_callback[i])
				{
					CanFrameData data;
					P_CANDriver p_can_driver = &g_can[i];
					while (_fetch_rx_frame(p_can_driver, &data))
						(*g_can_callback[i])(i, data);
				}
			}
		}
	}
}

void _can_output_task(void *p_arg)
{
	ks_os_thread_vfp_enable();
	U32 can_all_tx_flags = (1 << CAN_NUM) - 1;
	while(1)
	{
		U32 flags = ks_os_flag_wait(flag_handle_can_tx, can_all_tx_flags, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
		for (S32 i = 0; i < CAN_NUM; i++)
		{
			if (flags & (1 << i))
			{
				CanFrameData data;
				P_CANDriver p_can_driver = &g_can[i];
				if (_fetch_tx_frame(p_can_driver, &data))
				{
					_can_poll_send_frame(i, (data.state >> 7) & 0x1, (data.state >> 6) & 0x1, data.id, data.data, data.state & 0xF, 0);
				}
			}
		}
	}
}

void _can_send_frame(U32 id, U32 frame_format, U32 remote, U32 frame_id, const U8* p_data, S32 len)
{
	CanFrameData data;
	data.id = frame_id;
	len = (len > 8) ? 8 : len;
	data.state = ((frame_format & 0x1) << 7) | ((remote & 0x1) << 6) | (len & 0xF);
	for (S32 i = 0; i < len; i++)
		data.data[i] = p_data[i];
	P_CANDriver p_can_driver = &g_can[id];
	_put_to_tx_buffer(p_can_driver, &data);
	ks_os_flag_set_bits(flag_handle_can_tx, (1 << id), SET_FLAG_MODE_OR);
}

void _can_poll_send_frame(U32 id, U32 frame_format, U32 remote, U32 frame_id, const U8* p_data, S32 len, BOOL b_single_shot)
{
	P_CANDriver p_can_driver = &g_can[id];
	_can_low_level_send_frame(&p_can_driver->can_handle, frame_format, remote, frame_id, p_data, len, b_single_shot);
}

void _can_set_id_filter(U32 id, U32 acr, U32 amr, BOOL is_dual)
{
	P_CANDriver p_can_driver = &g_can[id];
	U8 split_acr[4];
	U8 split_amr[4];
	split_acr[0] = (acr >> 24) & 0xFF;
	split_acr[1] = (acr >> 16) & 0xFF;
	split_acr[2] = (acr >> 8) & 0xFF;
	split_acr[3] = acr & 0xFF;
	split_amr[0] = (amr >> 24) & 0xFF;
	split_amr[1] = (amr >> 16) & 0xFF;
	split_amr[2] = (amr >> 8) & 0xFF;
	split_amr[3] = amr & 0xFF;
	_can_low_level_set_acceptance_filter(&p_can_driver->can_handle, split_acr, split_amr, is_dual);
}

void _can_set_baud(U32 id, U32 baudrate)
{
	P_CANDriver p_can_driver = &g_can[id];
	_can_low_level_set_baud_rate(&p_can_driver->can_handle, baudrate);
}

void _can_reset(S32 id)
{
	P_CANDriver p_can_driver = &g_can[id];
	MCAN2_REG *p_can_reg = (MCAN2_REG*)(p_can_driver->can_handle.can_addr);
	p_can_reg->MOD |= 0x1;
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	p_can_reg->MOD &= ~0x1;
}

void _can_dump_info(S32 id)
{
	P_CANDriver p_can_driver = &g_can[id];
	MCAN2_REG *p_can_reg = (MCAN2_REG*)(p_can_driver->can_handle.can_addr);
	U8 mod = p_can_reg->MOD;
	U8 sr = p_can_reg->SR;
	U8 ier = p_can_reg->IER;
	U8 btr0 = p_can_reg->BTR0;
	U8 btr1 = p_can_reg->BTR1;
	U8 ocr = p_can_reg->OCR;
	U8 alc = p_can_reg->ALC;
	U8 ecc = p_can_reg->ECC;
	U8 ewlr = p_can_reg->EWLR;
	U8 rxerr = p_can_reg->RXERR;
	U8 txerr = p_can_reg->TXERR;
	U8 rmc = p_can_reg->RMC;
	U8 cdr = p_can_reg->CDR;
	char tmp_str[256];
	sprintf(tmp_str, "CAN Reg:\r\n"
			"MOD   %02X\r\n"
			"SR    %02X\r\n"
			"IER   %02X\r\n"
			"BTR0  %02X\r\n"
			"BTR1  %02X\r\n"
			"OCR   %02X\r\n"
			"ALC   %02X\r\n"
			"ECC   %02X\r\n"
			"EWLR  %02X\r\n"
			"RXERR %02X\r\n"
			"TXERR %02X\r\n"
			"RMC   %02X\r\n"
			"CDR   %02X\r\n"
			,
			mod,
			sr,
			ier,
			btr0,
			btr1,
			ocr,
			alc,
			ecc,
			ewlr,
			rxerr,
			txerr,
			rmc,
			cdr
			);
#ifdef CAN_DEBUG
		_uart_send_string(0, tmp_str);
#endif
}
