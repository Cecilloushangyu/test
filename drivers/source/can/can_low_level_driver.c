#include "can_low_level_driver.h"
#include <math.h>
#include "ks_os.h"

void _can_low_level_init(S32 id, CANHandle* p_can_handle)
{
	switch (id)
	{
	case 0: p_can_handle->can_addr = CAN0_BASE; break;
	case 1: p_can_handle->can_addr = CAN1_BASE; break;
	default: return;
	}
	MCAN2_REG *p_can_reg = (MCAN2_REG*)p_can_handle->can_addr;
	p_can_reg->MOD |= 0x1;
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	_can_low_level_config_interrupt(p_can_handle, 0x7);
	U8 acr[4] = {0,0,0,0};
	U8 amr[4] = {0xFF,0xFF,0xFF,0xFF};
	_can_low_level_set_acceptance_filter(p_can_handle, acr, amr, FALSE);
	_can_low_level_set_baud_rate(p_can_handle, 500000);
	p_can_reg->MOD &= ~0x1;
}

BOOL _can_low_level_calc_baud_param(DOUBLE div, S32 *p_brp, S32 *p_tseg1, S32 *p_tseg2)
{
	DOUBLE p = div / 64;
	if (p > 25)
		return FALSE;
	S32 brp = -1;
	DOUBLE min_residual = 1e9;
	for (S32 i = 0; i < 64; i++)
	{
		p = div / (i + 1);
		if (p >= 8 && p <= 25)
		{
			DOUBLE residual = fabs(p - round(p));
			if (residual < min_residual)
			{
				brp = i;
				min_residual = residual;
			}
		}
	}
	if (brp < 0)
		return FALSE;
	p = round(div / (brp + 1));
	if (p > 20)
	{
		*p_tseg1= 15;
		*p_tseg2 = p - 18;
	}
	else
	{
		*p_tseg2 = 2;
		*p_tseg1 = p - 5;
	}
	*p_brp = brp;
	return TRUE;
}
#if 1
void _can_low_level_set_baud_rate(CANHandle* p_can_handle, U32 baud)
{
	MCAN2_REG *p_can_reg = (MCAN2_REG*)p_can_handle->can_addr;
	DOUBLE clock = ks_os_get_apb_clock();
	p_can_handle->can_baud = baud;
	S32 brp = 0, tseg1 = 0, tseg2 = 0;
	if (_can_low_level_calc_baud_param(clock / 2. / baud, &brp, &tseg1, &tseg2))
	{
		U32 sjw = 2;
		U32 sam = 0;
		//	该寄存器仅可在复位模式下被写
		p_can_reg->MOD |= 0x1;
		asm("nop");
		asm("nop");
		asm("nop");
		asm("nop");
		p_can_reg->BTR0 = ((sjw & 0x3) << 6) | (brp & 0x3F);
		p_can_reg->BTR1 = ((sam & 0x1) << 7) | ((tseg2 & 0x7) << 4) | (tseg1 & 0xF);
		p_can_reg->MOD &= ~0x1;
	}
}
#else
void _can_low_level_set_baud_rate(CANHandle* p_can_handle, CANBaud baud)
{
	MCAN2_REG *p_can_reg = (MCAN2_REG*)p_can_handle->can_addr;
	DOUBLE clock = ks_os_get_apb_clock();
	U32 baud_rate = g_can_bauds[baud];
	p_can_handle->can_baud = baud_rate;
	U32 div = (U32)(clock / 2. / baud_rate);
	U32 sjw = 2;
	U32 sam = 1;
	U32 brp = 24;
	U32 tseg2 = 2;
	U32 tseg1 = div / (brp + 1) - 3 - tseg2;
	//	该寄存器仅可在复位模式下被写
	p_can_reg->MOD |= 0x1;
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	p_can_reg->BTR0 = ((sjw & 0x3) << 6) | (brp & 0x3F);
	p_can_reg->BTR1 = ((sam & 0x1) << 7) | ((tseg2 & 0x7) << 4) | (tseg1 & 0xF);
	p_can_reg->MOD &= ~0x1;
}
#endif
void _can_low_level_set_acceptance_filter(CANHandle* p_can_handle, U8 acr[4], U8 amr[4], BOOL b_is_dual_filter)
{
	MCAN2_REG *p_can_reg = (MCAN2_REG*)p_can_handle->can_addr;
	//	该寄存器仅可在复位模式下被写
	p_can_reg->MOD |= 0x1;
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	if (b_is_dual_filter)
		p_can_reg->MOD &= ~(1 << 3);
	else
		p_can_reg->MOD |= (1 << 3);
	for (S32 i = 0; i<4; i++)
	{
		p_can_reg->RXWINDOW[i] = acr[i];
		p_can_reg->RXWINDOW[i+4] = amr[i];
	}
	p_can_reg->MOD &= ~0x1;
}

void _can_low_level_read_rx_fifo(CANHandle* p_can_handle, U8 dest_buffer[13])
{
	MCAN2_REG *p_can_reg = (MCAN2_REG*)p_can_handle->can_addr;
	for (S32 i = 0; i<13; i++)
		dest_buffer[i] = p_can_reg->RXWINDOW[i];
	p_can_reg->CMR |= 1 << 2;
}

U32 _can_low_level_read_interrupt(CANHandle* p_can_handle)
{
	MCAN2_REG *p_can_reg = (MCAN2_REG*)p_can_handle->can_addr;
	return p_can_reg->IR;
}

void _can_low_level_config_interrupt(CANHandle* p_can_handle, U8 enable_int)
{
	MCAN2_REG *p_can_reg = (MCAN2_REG*)p_can_handle->can_addr;
	p_can_reg->IER = enable_int;
}

U32 _can_get_sr(MCAN2_REG* p_can_reg)
{
	return p_can_reg->SR;
}

void _can_low_level_send_frame(CANHandle* p_can_handle, S32 frame_format, S32 remote, U32 frame_id, const U8* p_data, S32 len, BOOL b_single_shot)
{
	MCAN2_REG *p_can_reg = (MCAN2_REG*)p_can_handle->can_addr;
	U8 tx_ready_flag;
	do
	{
		tx_ready_flag = (_can_get_sr(p_can_reg) >> 2) & 0x1;
	} while (tx_ready_flag == 0);
	if (len > 8)
		len = 8;
	U8 tx_frame_info = ((frame_format & 0x1) << 7) | ((remote & 0x1) << 6) | (len & 0xF);
	p_can_reg->RXWINDOW[0] = tx_frame_info;
	if (frame_format == 0)
	{
		// SFF
		p_can_reg->RXWINDOW[1] = (frame_id >> 3) & 0xFF;
		p_can_reg->RXWINDOW[2] = (frame_id & 0x7) << 5;
		for (S32 i = 0; i < len; i++)
		{
			p_can_reg->RXWINDOW[3 + i] = p_data[i];
		}
	}
	else
	{
		// EFF
		p_can_reg->RXWINDOW[1] = (frame_id >> 21) & 0xFF;
		p_can_reg->RXWINDOW[2] = (frame_id >> 13) & 0xFF;
		p_can_reg->RXWINDOW[3] = (frame_id >> 5) & 0xFF;
		p_can_reg->RXWINDOW[4] = (frame_id & 0x1F) << 3;
		for (S32 i = 0; i < len; i++)
		{
			p_can_reg->RXWINDOW[5 + i] = p_data[i];
		}
	}
	if (b_single_shot)
		p_can_reg->CMR = 0x3;
	else
		p_can_reg->CMR = 0x1;
}
