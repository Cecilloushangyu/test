#include "uart_low_level_driver.h"
#include "dw_uart_define.h"
#include "ks_os.h"




int _uart_low_level_init(int port, pUARTHandle p_uart_handle)
{
	switch (port)
	{
	case 0: p_uart_handle->uart_addr = UART0_BASE; break;
	case 1: p_uart_handle->uart_addr = UART1_BASE; break;
	case 2: p_uart_handle->uart_addr = UART2_BASE; break;
	case 3: p_uart_handle->uart_addr = UART3_BASE; break;
	default: return 1;
	}

	//	reset UART
	DW_UART_REG *uart_regs = (DW_UART_REG *) p_uart_handle->uart_addr;
	uart_regs->SRR = 0x7;
	asm("nop");
	asm("nop");
	asm("nop");
	asm("nop");
	uart_regs->SRR = 0x0;

	_uart_low_level_set_baud_rate(p_uart_handle, 115200);
	return 0;
}

void _uart_low_level_set_baud_rate(pUARTHandle uart, unsigned int baud_rate)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	DOUBLE clock = ks_os_get_apb_clock();
	uart->uart_baud = baud_rate;

	DOUBLE d = clock / (baud_rate * 16.0);
	unsigned int divisor  = (unsigned int)d;
	unsigned int dlf = (unsigned int)((d - divisor) * 16.0);

	uart_regs->SRR = 0x01;
	uart_regs->LCR = DW_UART_LCR_8N1; // 8n1

	uart_regs->LCR |= 0x80;
	uart_regs->DATA.DLL = divisor & 0xFF;
	uart_regs->IER_DLH.DLH = (divisor>>8)&0xFF;
	uart_regs->Reserved_F0_AC[5] = dlf; // DLF
	uart_regs->LCR &=0x7F;
}

void _uart_low_level_set_stop_bits(pUARTHandle uart,  uint32_t stop_bits)
{
    int32_t ret;
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	uart->stop_bits = stop_bits;

    //when data length is 5 bits, use dw_uart_config_stop_bits_2 will be 1.5 stop bits
    if (stop_bits == UART_STOP_BITS_1) {
          uart_regs->LCR &= ~(0x1U << 2);
    } else if (stop_bits == UART_STOP_BITS_2) {

		uart_regs->LCR |= 0x1U << 2;
    }

}


void  _uart_low_level_set_parity(pUARTHandle uart, uint32_t parity)
{
    int32_t ret;
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	uart->parity = parity;


    switch (parity)
    {
        case UART_PARITY_NONE:
             uart_regs->LCR &= (~ (0x1U << 3));
            break;
        case UART_PARITY_ODD:
         	uart_regs->LCR |= 0x1U << 3;
        	uart_regs->LCR &= ~(0x1U << 4);
		    uart_regs->LCR &= ~(0x1U << 5);
            break;
        case UART_PARITY_EVEN:
            uart_regs->LCR |= 0x1U << 3;
        	uart_regs->LCR |= 0x1U << 4;
			uart_regs->LCR &= ~(0x1U << 5);
            break;
		case UART_PARITY_MARK:
			uart_regs->LCR |= 0x1U << 3;
			uart_regs->LCR &= ~(0x1U << 4);
			uart_regs->LCR |= 0x1U << 5;
			break;
		case UART_PARITY_SPACE:
			uart_regs->LCR |= 0x1U << 3;
			uart_regs->LCR |= 0x1U << 4;
			uart_regs->LCR |= 0x1U << 5;
			break;

        default:
            ks_exception_assert(0);
            break;
    }

}



void _uart_low_level_set_data_bits(pUARTHandle uart,  uint32_t data_width)
{

	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	uart->data_width = data_width;

    uart_regs->LCR &= 0xFCU;
    uart_regs->LCR |= (data_width - 5U);


}

void _uart_low_level_setup_fifo(pUARTHandle uart, int enable)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;

	//	reset tx/rx fifo, set FIFOE to 0
	uart_regs->IIR_FCR.FCR = DW_UART_FCR_XFIFOR | DW_UART_FCR_RFIFOR;

	if (enable == 1)
	{
		//	rx trigger at FIFO 2 less than full
		//	tx trigger at 2 char in FIFO
		//uart_regs->IIR_FCR.FCR = DW_UART_FCR_RCVR_1P2 | DW_UART_FCR_TET_2 | DW_UART_FCR_FIFOE;
		uart->fcr_reg = DW_UART_FCR_RCVR_1P2 | DW_UART_FCR_TET_0 | DW_UART_FCR_FIFOE|DW_UART_FCR_DMAM1;
		uart_regs->IIR_FCR.FCR = uart->fcr_reg;

	}
}

void _uart_low_level_config_tx_fifo_half_full(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	uart->fcr_reg &= ~(3<<4);
	uart->fcr_reg |= DW_UART_FCR_TET_1P2;
	uart_regs->IIR_FCR.FCR = uart->fcr_reg;

}

void _uart_low_level_config_tx_fifo_char_2(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;

	uart->fcr_reg &= ~(3<<4);
	uart->fcr_reg |= DW_UART_FCR_TET_2;
	uart_regs->IIR_FCR.FCR = uart->fcr_reg ;

}


void _uart_low_level_config_tx_fifo_empty(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;

	uart->fcr_reg &= ~(3<<4);
	uart->fcr_reg |= DW_UART_FCR_TET_0;
	uart_regs->IIR_FCR.FCR = uart->fcr_reg;

}


void _uart_low_level_config_rx_fifo_half_full(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;

	uart->fcr_reg  &= ~(0x3 << 6);
	uart->fcr_reg |= DW_UART_FCR_RCVR_1P2;
	uart_regs->IIR_FCR.FCR = uart->fcr_reg ;

}

void _uart_low_level_config_rx_fifo_1P4(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;

	uart->fcr_reg  &= ~(0x3 << 6);
	uart->fcr_reg |= DW_UART_FCR_RCVR_1P4;
	uart_regs->IIR_FCR.FCR = uart->fcr_reg ;


}

void _uart_low_level_config_rx_fifo_char_1(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;

	uart->fcr_reg  &= ~(0x3 << 6);
	uart->fcr_reg |= DW_UART_FCR_RCVR_1;
	uart_regs->IIR_FCR.FCR = uart->fcr_reg ;
}


int _uart_low_level_get_fifo_size(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	int mode = (uart_regs->CPR >> 16) & 0xFF;
	switch (mode)
	{
	case 0x0: return 0;
	case 0x1: return 16;
	case 0x2: return 32;
	case 0x4: return 64;
	case 0x8: return 128;
	case 0x10: return 256;
	case 0x20: return 512;
	case 0x40: return 1024;
	case 0x80: return 2048;
	default: return -1;
	}
}

int _uart_low_level_get_tx_fifo_level(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	return uart_regs->TFL;
}

int _uart_low_level_get_rx_fifo_level(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	return uart_regs->RFL;
}

void _uart_low_level_write_tx_fifo(pUARTHandle uart, char *data, int len)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	for (int i=0; i<len; i++)
	{
		uart_regs->DATA.THR = data[i];
	}
}

void _uart_low_level_read_rx_fifo(pUARTHandle uart, char *data, int len)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	for (int i=0; i<len; i++)
	{
		data[i] = uart_regs->DATA.RBR;
	}
}

void _uart_low_level_config_tx_int(pUARTHandle uart, int enable)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	uart_regs->LCR &= ~0x80;
	if (enable)
		uart_regs->IER_DLH.IER |= ((1 << 1) | (1 << 7));
	else
		uart_regs->IER_DLH.IER &= ~(1 << 1);
}

void _uart_low_level_config_rx_int(pUARTHandle uart, int enable)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	uart_regs->LCR &= ~0x80;
	
	if (enable)
		uart_regs->IER_DLH.IER |= (1 << 0);
	else
		uart_regs->IER_DLH.IER &= ~(1 << 0);
}


UARTIntType _uart_low_level_get_int_type(pUARTHandle uart)
{
	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	switch (uart_regs->IIR_FCR.IIR & 0xF)
	{
	case 0x1: return UART_INT_NONE;
	case 0x2: return UART_INT_TX_EMPTY;
	case 0x4: return UART_INT_RX_VALID;
	case 0xc: return UART_INT_RX_TIMEOUT;
	default: return UART_INT_UNKNOWN;
	}
}

void _uart_low_level_print(pUARTHandle uart, char *data, int len)
{
	volatile int fifoSize, fifoUsed;
	fifoSize = _uart_low_level_get_fifo_size(uart);
	while (len > 0)
	{
		do
		{
			fifoUsed = _uart_low_level_get_tx_fifo_level(uart);
		} while (fifoUsed != 0);

		int cur_len = (len > fifoSize) ? fifoSize : len;
		_uart_low_level_write_tx_fifo(uart, data, cur_len);
		len -= cur_len;
		data += cur_len;
	}
}

char _uart_low_level_get_byte(pUARTHandle uart)
{
	char rcv_byte;
	do
	{
		int numChar = _uart_low_level_get_rx_fifo_level(uart);
		if (numChar != 0)
		{
			_uart_low_level_read_rx_fifo(uart, &rcv_byte, 1);
			return rcv_byte;
		}
	} while (1);
}


int  _uart_low_level_get_dma_tx_handshaking(pUARTHandle p_uart_handle){
	int handshaking = 0;
	U32 uart_addr = p_uart_handle->uart_addr ;
	switch (uart_addr)
	{
		case UART0_BASE: handshaking = DMA_UART0_TX; break;
		case UART1_BASE: handshaking = DMA_UART1_TX; break;
		case UART2_BASE: handshaking = DMA_UART2_TX ; break;
		case UART3_BASE: handshaking = DMA_UART3_TX ; break;
		default: return 1;
	}
	
	return handshaking;
}

int  _uart_low_level_get_dma_rx_handshaking(pUARTHandle p_uart_handle){
	int handshaking = 0;
	U32 uart_addr = p_uart_handle->uart_addr ;
	switch (uart_addr)
	{
		case UART0_BASE: handshaking = DMA_UART0_RX; break;
		case UART1_BASE: handshaking = DMA_UART1_RX; break;
		case UART2_BASE: handshaking = DMA_UART2_RX ; break;
		case UART3_BASE: handshaking = DMA_UART3_RX ; break;
		default: return 1;
	}
		
	return handshaking;
}





int  _uart_low_level_config_tx_dma_channel(OSHandle dma_handle,pUARTHandle uart,int32_t ch,dma_event_cb callback,void* arg){

	dma_config_t config;
    config.src_inc  = DMA_ADDR_INC;
    config.dst_inc  = DMA_ADDR_CONSTANT;
    config.src_endian = DMA_ADDR_LITTLE;
    config.dst_endian = DMA_ADDR_LITTLE;

	config.src_tw   = DMA_DATAWIDTH_SIZE8;
    config.dst_tw   = DMA_DATAWIDTH_SIZE8;
	
    config.type     = DMA_MEM2PERH;
    config.ch_mode  = DMA_HANDSHAKING_HARDWARE;
	config.hs_if = _uart_low_level_get_dma_tx_handshaking(uart);
	config.burst_len = 0;


    int ret = ks_driver_dma_config_channel(dma_handle, ch, &config, callback,arg);
    if (ret < 0) {
        kprintf("ks_driver_dma_config_channel error %d  \r\n",ret);
        return ret;
    }
	
	return ret;

}


int  _uart_low_level_config_rx_dma_channel(OSHandle dma_handle,pUARTHandle uart,int32_t ch,dma_event_cb callback,void* arg){

	dma_config_t config;
    config.src_inc  = DMA_ADDR_CONSTANT ;
    config.dst_inc  = DMA_ADDR_INC;
    config.src_endian = DMA_ADDR_LITTLE;
    config.dst_endian = DMA_ADDR_LITTLE;

	config.src_tw   = DMA_DATAWIDTH_SIZE8;
    config.dst_tw   = DMA_DATAWIDTH_SIZE8;
	
    config.type     = DMA_PERH2MEM;
    config.ch_mode  = DMA_HANDSHAKING_HARDWARE;
	config.hs_if = _uart_low_level_get_dma_rx_handshaking(uart);
	config.burst_len = 2;
	

    int ret = ks_driver_dma_config_channel(dma_handle, ch, &config, callback,arg);
    if (ret < 0) {
        kprintf("ks_driver_dma_config_channel error %d  \r\n",ret);
        return ret;
    }
	
	return ret;

}


int  _uart_low_level_start_tx_dma(OSHandle dma_handle ,pUARTHandle uart,int32_t ch, uint8_t*data, int len){

	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	uart->dma_tx_channel = ch;
		
    int ret = ks_driver_dma_start(dma_handle, ch, (void*)data, (void*)&uart_regs->DATA.THR, (uint32_t)len);
    if (ret < 0) {
        kprintf("ks_driver_dma_start error  %d \r\n",ret);
    }
	return ret ;
}

int  _uart_low_level_start_rx_dma(OSHandle dma_handle ,pUARTHandle uart,int32_t ch, uint8_t*data, int len){

	DW_UART_REG *uart_regs = (DW_UART_REG *) uart->uart_addr;
	uart->dma_rx_channel = ch;
    int ret = ks_driver_dma_start(dma_handle, ch, (void*)&uart_regs->DATA.RBR ,(void*)data, (uint32_t)len);
    if (ret < 0) {
        kprintf("ks_driver_dma_start error  %d \r\n",ret);
    }
	return ret ;
}

