#ifndef _UART_LOW_LEVEL_DRIVER_H_
#define _UART_LOW_LEVEL_DRIVER_H_

#include "ks_driver.h"
#include "ks_dma.h"



// UART0/1/2/3
#define	UART_RBR_OFS			0x00	/* Receive Buffer Register */
#define	UART_THR_OFS			0x00	/* Transmit Holding Register */
#define	UART_DLL_OFS			0x00	/* Divisor Latch Low */
#define	UART_DLH_OFS			0x04	/* Divisor Latch High */
#define	UART_IER_OFS			0x04	/* Interrupt Enable Register */
#define	UART_IIR_OFS			0x08	/* Interrupt Identification Register */
#define	UART_FCR_OFS			0x08	/* FIFO Control Register */
#define	UART_LCR_OFS			0x0C	/* Line Control Register */
#define	UART_MCR_OFS			0x10	/* Modem Control Register */
#define	UART_LSR_OFS			0x14	/* Line Status Register */
#define	UART_SCR_OFS			0x1C	/* Scratchpad Register */
#define	UART_USR_OFS			0x7C	/* Uart Status Register */
#define	UART_TFL_OFS			0x80	/* Transmit FIFO Level */
#define	UART_RFL_OFS			0x84	/* Receive FIFO Level */
#define	UART_SRR_OFS			0x88	/* Software Reset Register */
#define	UART_SBCR_OFS			0x90	/* Shadow Break Control Register */
#define	UART_SDMAM_OFS			0x94	/* Shadow DMA Mode */
#define	UART_SFE_OFS			0x98	/* Shadow FIFO Enable */
#define	UART_SRT_OFS			0x9C	/* Shadow RCVR Trigger */
#define	UART_STET_OFS			0xA0	/* Shadow TX Empty Trigger */
#define	UART_HTX_OFS			0xA4	/* Halt TX */
#define	UART_DMASA_OFS			0xA8	/* DMA Software Acknowledge */
#define	UART_DLF_OFS			0xC0	/* Divisor Latch Fraction */

#define UART_CTRL(n, x)			HW_REG_WR(UART0_BASE + n*0x10000 + x)
#define	UART_READ(n, x, value)	HW_REG_RD(UART0_BASE + n*0x10000 + x, value)




typedef struct
{
    U32 uart_addr;
    U32 uart_id;
    U32 uart_baud;
	U32 data_width;
	U32 parity;
	U32 stop_bits;
	U32 fcr_reg;
	U32 dma_tx_status;
	U32 dma_rx_status;
	U32 dma_tx_channel;
	U32 dma_rx_channel;
	U32 tx_dma_ok_count;
	U32 tx_dma_req_count;
	U32 rx_dma_ok_count;
	U32 rx_dma_br_count;
	U32 rx_dma_lost_count;
	U32 tx_fifo_count;
	U32 rx_fifo_count;
	U32 rx_fifo_lost;
	U32 dma_failure;
	U32 dma_erro;

} UARTHandle, *pUARTHandle;

typedef enum
{
	UART_INT_NONE = 0,
	UART_INT_TX_EMPTY,
	UART_INT_RX_VALID,
	UART_INT_RX_TIMEOUT,
	UART_INT_UNKNOWN
} UARTIntType;

int _uart_low_level_init(int port, pUARTHandle p_uart_handle);
void _uart_low_level_set_baud_rate(pUARTHandle uart, unsigned int baud);
void _uart_low_level_set_data_bits(pUARTHandle uart,  uint32_t data_width);
void  _uart_low_level_set_parity(pUARTHandle uart, uint32_t parity);
void _uart_low_level_set_stop_bits(pUARTHandle uart,  uint32_t stop_bits);
void _uart_low_level_setup_fifo(pUARTHandle uart, int enable);
int _uart_low_level_get_fifo_size(pUARTHandle uart);
int _uart_low_level_get_tx_fifo_level(pUARTHandle uart);
int _uart_low_level_get_rx_fifo_level(pUARTHandle uart);
void _uart_low_level_write_tx_fifo(pUARTHandle uart, char *data, int len);
void _uart_low_level_read_rx_fifo(pUARTHandle uart, char *data, int len);
void _uart_low_level_config_tx_int(pUARTHandle uart, int enable);
void _uart_low_level_config_rx_int(pUARTHandle uart, int enable);
UARTIntType _uart_low_level_get_int_type(pUARTHandle uart);
void _uart_low_level_print(pUARTHandle uart, char *data, int len);
char _uart_low_level_get_byte(pUARTHandle uart);
int  _uart_low_level_config_tx_dma_channel(OSHandle dma_handle,pUARTHandle uart,	int32_t ch,dma_event_cb callback,void* arg);
int  _uart_low_level_start_tx_dma(OSHandle dma_handle ,pUARTHandle uart,int32_t ch, uint8_t*data, int len);
int  _uart_low_level_config_rx_dma_channel(OSHandle dma_handle,pUARTHandle uart,int32_t ch,dma_event_cb callback,void* arg);
int  _uart_low_level_start_rx_dma(OSHandle dma_handle ,pUARTHandle uart,int32_t ch, uint8_t*data, int len);
void _uart_low_level_config_rx_fifo_half_full(pUARTHandle uart);
void _uart_low_level_config_rx_fifo_char_1(pUARTHandle uart);



#endif
