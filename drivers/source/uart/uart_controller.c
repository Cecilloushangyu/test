
#include "ks_gpio.h"
#include "ks_sysctrl.h"
#include "uart_controller.h"
#include "uart_low_level_driver.h"
#include "ks_os.h"
#include <string.h>
#include "ks_taskdef.h"
#include "ks_dma.h"
#include "ks_shell.h"
#include "ks_cache.h"
#include "dw_uart_define.h"
#include "ringbuffer.h"
#define UART_NUM		4

// protocol
#define MAX_PROTOCOL_NUMBER 	4
static UartEntry g_uart_parser[UART_NUM][MAX_PROTOCOL_NUMBER];
static int g_handle_serial_port[UART_NUM];

// uart entry
#define STACK_SIZE_UART_ENTRY		768
static OSHandle flag_handle_uart_rx;
static OSHandle thread_handle_uart_entry;
static int thread_stack_uart_entry[STACK_SIZE_UART_ENTRY];

// uart print
#define UART_TASK_PRINT_SIZE		768
static OSHandle flag_handle_uart_tx;
static OSHandle thread_handle_uart_print;
static int thread_stack_uart_print[UART_TASK_PRINT_SIZE];

void _uart_print_task(void *p_arg);
extern S32 ks_bb_get_debug_port();



// global uart
#define UART_BUF_RX_SIZE			8192

#define UART_BUF_TX_SIZE			64*1024


#define UART_TX_DMA_SIZE			2048
#define UART_RX_DMA_SIZE			2048


typedef struct
{
  int enable;
  U8* rx_buf; 	// The receive buffer
  U8* tx_buf; 	// The transmit buffer
  int rx_front;
  int rx_rear;
  S32 tx_read;
  S32 tx_write;
  S32 tx_valid;
  U32 rx_count;
  U32 tx_count;
  U32 tx_lost;
  U32 tx_ring;
  U32 dma_tx_len;
  U32 dma_rx_len;
  U8 *dma_buffer_tx;
  U8 *dma_buffer_rx;
  ringbuf_t	tx_ringbuffer;
  ringbuf_t	rx_ringbuffer;
  U8 tx_dma_enable;
  U8 rx_dma_enable;
  S32 tx_dma_ch;
  S32 rx_dma_ch;
  OSHandle mux;
  UARTHandle hUART;
} UartDriver, *pUartDriver;

static UartDriver g_uart[UART_NUM];
static U32 UART_START_INDEX[UART_NUM];
static U8 g_tx_buffer[UART_BUF_TX_SIZE*UART_NUM] __attribute__ ((aligned (64)));
static U8 g_rx_buffer[UART_BUF_RX_SIZE*UART_NUM] __attribute__ ((aligned (64)));

static U8 g_dma_rx_buffer[UART_RX_DMA_SIZE*UART_NUM] __attribute__ ((aligned (64)));

void _uart_entry_task(void *p_arg);
void _uart_isr_handler(void *arg);
int _get_tx_buffer_length(pUartDriver p_uart);
int _en_tx_buffer(pUartDriver p_uart, const U8* p_data, S32 len);
void _de_tx_buffer(pUartDriver p_uart, U8* p_buffer, S32 len);
void _uart_dma_event_cb(dma_channel_context* ch_ctx,int32_t ch, dma_event_e event);
int _get_tx_dma_buffer(pUartDriver p_uart, U8** p_bufferout, S32 len);
int _get_tx_dma_buffer_len(pUartDriver p_uart, S32 len);

void _update_tx_buffer_offset(pUartDriver p_uart,  S32 len);
int _get_tx_fifo_buffer(pUartDriver p_uart, U8** p_bufferout, S32 len);

int _uart_dma_recieve_init(pUartDriver p_uart,int dma_len);


static int uart_ctx_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t uart_id = 0;
	char  pstr[1024]; 
	int len ;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &uart_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}
	
	pUartDriver p_uart = &g_uart[uart_id];

	memset(pstr,0,1024);

	int  ofs = 0;
	ofs += sprintf( pstr + ofs, "\r\n");
    ofs += sprintf( pstr + ofs, "uart_id : %d \r\n",p_uart->hUART.uart_id);
    ofs += sprintf( pstr + ofs, "uart_baud : %d \r\n",p_uart->hUART.uart_baud);
    ofs += sprintf( pstr + ofs, "data_width : %d \r\n",p_uart->hUART.data_width);
    ofs += sprintf( pstr + ofs, "parity : %d \r\n",p_uart->hUART.parity);
    ofs += sprintf( pstr + ofs, "stop_bits : %d \r\n",p_uart->hUART.stop_bits);
    ofs += sprintf( pstr + ofs, "dma_failure : %d \r\n",p_uart->hUART.dma_failure);
    ofs += sprintf( pstr + ofs, "dma_erro : %d \r\n",p_uart->hUART.dma_erro);
 	ofs += sprintf( pstr + ofs, "tx_lost : %d \r\n",p_uart->tx_lost );
    ofs += sprintf( pstr + ofs, "tx_ring : %d \r\n",p_uart->tx_ring );
    ofs += sprintf( pstr + ofs, "tx_fifo_count : %d \r\n",p_uart->hUART.tx_fifo_count);
	ofs += sprintf( pstr + ofs,	"tx_valid : %d \r\n",p_uart->tx_valid );
    ofs += sprintf( pstr + ofs, "tx_count : %d \r\n",p_uart->tx_count);
    ofs += sprintf( pstr + ofs, "rx_count : %d \r\n",p_uart->rx_count);
	ofs += sprintf( pstr + ofs, "rx_fifo_count : %d \r\n",p_uart->hUART.rx_fifo_count);
	ofs += sprintf( pstr + ofs, "rx_fifo_lost : %d \r\n",p_uart->hUART.rx_fifo_lost);
	ofs += sprintf( pstr + ofs, "tx_dma_req_count : %d \r\n",p_uart->hUART.tx_dma_req_count);
	ofs += sprintf( pstr + ofs, "tx_dma_ok_count : %d \r\n",p_uart->hUART.tx_dma_ok_count);
	ofs += sprintf( pstr + ofs, "rx_dma_br_count : %d \r\n",p_uart->hUART.rx_dma_br_count);
	ofs += sprintf( pstr + ofs, "rx_dma_ok_count : %d \r\n",p_uart->hUART.rx_dma_ok_count);
	ofs += sprintf( pstr + ofs, "rx_dma_lost_count : %d \r\n",p_uart->hUART.rx_dma_lost_count);

	len = strlen(pstr);
	ks_shell_printf(ctx->uart_id,pstr,len);


    return 0;
}

static int uart_clear_ctx(pUartDriver p_uart){

	p_uart->hUART.tx_dma_ok_count = 0;
	p_uart->hUART.tx_dma_req_count = 0;
	p_uart->hUART.rx_dma_ok_count = 0;
	p_uart->hUART.rx_dma_br_count = 0;
	p_uart->hUART.tx_fifo_count = 0;
	p_uart->hUART.rx_fifo_count = 0;
	p_uart->rx_count = 0;
	p_uart->tx_count = 0;
	p_uart->tx_lost = 0;
	p_uart->tx_ring = 0;

	return 0 ;
}

static int uart_clear_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
	uint32_t uart_id = 0;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &uart_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}

	pUartDriver p_uart = &g_uart[uart_id];
	uart_clear_ctx(p_uart);

    return 0;
}



static int uart_reg_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
    uint32_t uart_id = 0;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &uart_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}
	
	pUartDriver p_uart = &g_uart[uart_id];
	DW_UART_REG *uart_regs = (DW_UART_REG *) p_uart->hUART.uart_addr;
	
	ks_shell_printf(ctx->uart_id,"\r\n");
	ks_shell_printf(ctx->uart_id,"uart_id : %d \r\n",p_uart->hUART.uart_id);
	ks_shell_printf(ctx->uart_id,"IER : %x \r\n",uart_regs->IER_DLH.IER);
	ks_shell_printf(ctx->uart_id,"IIR : %x \r\n",uart_regs->IIR_FCR.IIR);
	ks_shell_printf(ctx->uart_id,"FCR : %x \r\n",p_uart->hUART.fcr_reg);

	ks_shell_printf(ctx->uart_id,"USR : %x \r\n",uart_regs->USR);
	ks_shell_printf(ctx->uart_id,"TFL : %x \r\n",uart_regs->TFL);
	ks_shell_printf(ctx->uart_id,"RFL : %x \r\n",uart_regs->RFL);

    return 0;
}

static int uart_dma_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
	uint32_t uart_id = 0;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &uart_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}

	pUartDriver p_uart = &g_uart[uart_id];
	
	ks_shell_printf(ctx->uart_id,"\r\n");
	ks_shell_printf(ctx->uart_id,"uart_id : %d \r\n",p_uart->hUART.uart_id);
	ks_shell_printf(ctx->uart_id,"tx_dma_enable : %d \r\n",p_uart->tx_dma_enable );
	ks_shell_printf(ctx->uart_id,"tx_dma_ch : %d \r\n",p_uart->tx_dma_ch );
	ks_shell_printf(ctx->uart_id,"dma_tx_status : %d \r\n",p_uart->hUART.dma_tx_status );
	ks_shell_printf(ctx->uart_id,"rx_dma_enable : %d \r\n",p_uart->rx_dma_enable );
	ks_shell_printf(ctx->uart_id,"rx_dma_ch : %d \r\n",p_uart->rx_dma_ch );
	ks_shell_printf(ctx->uart_id,"dma_rx_status : %d \r\n",p_uart->hUART.dma_rx_status );


    return 0;
}


static int uart_ring_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
	uint32_t uart_id = 0;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &uart_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}

	pUartDriver p_uart = &g_uart[uart_id];
	
	ks_shell_printf(ctx->uart_id,"\r\n");
	ks_shell_printf(ctx->uart_id,"uart_id : %d \r\n",p_uart->hUART.uart_id);
	ks_shell_printf(ctx->uart_id,"buffer_s : %x \r\n",&p_uart->rx_ringbuffer.buffer[0]);
	ks_shell_printf(ctx->uart_id,"buffer_e : %x \r\n",&p_uart->rx_ringbuffer.buffer[p_uart->rx_ringbuffer.length]);
	ks_shell_printf(ctx->uart_id,"length : %d \r\n",p_uart->rx_ringbuffer.length );
	ks_shell_printf(ctx->uart_id,"ridx : %d \r\n",p_uart->rx_ringbuffer.ridx );
	ks_shell_printf(ctx->uart_id,"widx : %d \r\n",p_uart->rx_ringbuffer.widx);

	ks_shell_printf(ctx->uart_id,"buffer[ridx] : %x \r\n",&p_uart->rx_ringbuffer.buffer[p_uart->rx_ringbuffer.ridx]);
	ks_shell_printf(ctx->uart_id,"buffer[widx] : %x \r\n",&p_uart->rx_ringbuffer.buffer[p_uart->rx_ringbuffer.widx]);

    return 0;
}


static cmd_proc_t uart_driver_cmds[] = {
	{.cmd = "uart_ctx", .fn = uart_ctx_cmd,  .help = "uart_ctx <uartid>"},
	{.cmd = "uart_clear", .fn = uart_clear_cmd,  .help = "uart_clear <uartid>"},
	{.cmd = "uart_reg", .fn = uart_reg_cmd,  .help = "uart_reg <uartid>"},
	{.cmd = "uart_dma", .fn = uart_dma_cmd,  .help = "uart_dma <uartid>"},
	{.cmd = "uart_ring", .fn = uart_ring_cmd,  .help = "uart_ring <uartid>"},

};




int _get_uart_irq_vector(int uart_id)
{
	int irq_vec_num;
	switch(uart_id)
	{
	case 0: irq_vec_num = IRQ_VEC_UART0; break;
	case 1: irq_vec_num = IRQ_VEC_UART1; break;
	case 2: irq_vec_num = IRQ_VEC_UART2; break;
	case 3: irq_vec_num = IRQ_VEC_UART3; break;
	default: irq_vec_num = IRQ_VEC_UART0; break;
	}
	return irq_vec_num;
}

S32 _uart_init(int uart_id, int baud,int sel)
{

	static int uart_task_created = 0;
	UART_START_INDEX[uart_id] = uart_id;
	char buffer[30];
	int mode;
	int tx_dma_enable;
	int rx_dma_enable;

	if(uart_id > 3 ) return DRV_ERROR_PARAMETER;


	mode = (sel&0x0F);
	
	tx_dma_enable = (sel & TX_DMA_ENABLE);
	rx_dma_enable = (sel & RX_DMA_ENABLE);

	if(uart_id == 1)
	{
		ks_driver_sysctrl_set_mux_sel(GPIOA_17,MUX_V_FUN1);
		ks_driver_sysctrl_set_mux_sel(GPIOA_18,MUX_V_FUN1);
		ks_driver_sysctrl_set_fuction_sel(CAN0_FUNC_SEL,0);
		ks_driver_sysctrl_set_clock_enable(UART1_CLK_EN,1);
		ks_driver_sysctrl_set_dev_reset(UART1_RST,1);
	}
	else if(uart_id == 2)
	{
		ks_driver_sysctrl_set_mux_sel(GPIOA_19,MUX_V_FUN1);
		ks_driver_sysctrl_set_mux_sel(GPIOA_20,MUX_V_FUN1);
		ks_driver_sysctrl_set_fuction_sel(CAN1_FUNC_SEL,0);
		ks_driver_sysctrl_set_clock_enable(UART2_CLK_EN,1);
		ks_driver_sysctrl_set_dev_reset(UART2_RST,1);

	}
	else if(uart_id == 3)
	{
		if(mode==0){
			ks_driver_sysctrl_set_mux_sel(GPIOA_12,MUX_V_FUN2);
			ks_driver_sysctrl_set_mux_sel(GPIOA_13,MUX_V_FUN2);

		}else{
			ks_driver_sysctrl_set_mux_sel(GPIOA_4,MUX_V_FUN3);
			ks_driver_sysctrl_set_mux_sel(GPIOA_5,MUX_V_FUN3);
		}
		
		ks_driver_sysctrl_set_clock_enable(UART3_CLK_EN,1);
		ks_driver_sysctrl_set_dev_reset(UART3_RST,1);
	
	}
	else
	{

	}
	pUartDriver p_uart = &g_uart[uart_id];

	memset(p_uart, 0, sizeof(UartDriver));

	p_uart->enable = 1;
	p_uart->tx_buf = (U8*) (((U32)g_tx_buffer) + uart_id * UART_BUF_TX_SIZE);
	p_uart->rx_buf = (U8*) (((U32)g_rx_buffer) + uart_id * UART_BUF_RX_SIZE);
	
	
	//init ring buf
	ringbuffer_create(&p_uart->rx_ringbuffer, (char*)p_uart->rx_buf, UART_BUF_RX_SIZE);

	p_uart->hUART.uart_id = uart_id;
	memset(buffer,0,sizeof(buffer));
	sprintf(buffer,"uart%d mux",uart_id);
	ks_os_mutex_create(&p_uart->mux, buffer); 

	_uart_low_level_init(UART_START_INDEX[uart_id], &p_uart->hUART);

	_uart_config(uart_id, baud, UART_DATA_BITS_8,UART_PARITY_NONE,UART_STOP_BITS_1);

	int irq_vec_num = _get_uart_irq_vector(UART_START_INDEX[uart_id]);

	ks_os_irq_create(irq_vec_num, _uart_isr_handler, (void*)uart_id);
	ks_os_irq_enable(irq_vec_num);
	ks_os_irq_map_target(irq_vec_num, 1);

	_uart_low_level_config_rx_int(&p_uart->hUART, 1);
   
	for (S32 j = 0; j < MAX_PROTOCOL_NUMBER; j++)
	{
		g_uart_parser[uart_id][j] = 0;
	}
	g_handle_serial_port[uart_id] = 1;

	
	//dma channel 提前分配
	if(tx_dma_enable||rx_dma_enable){
		
		p_uart->tx_dma_enable = tx_dma_enable;
		p_uart->rx_dma_enable = rx_dma_enable;
	
		OSHandle handle = ks_driver_dma_init();
		
		if(p_uart->tx_dma_enable){
			p_uart->tx_dma_ch = ks_driver_dma_bind_channel( handle);
			if(p_uart->tx_dma_ch == -1) ks_exception_assert(0);
		}
		
		if(p_uart->rx_dma_enable){
			if(p_uart->dma_rx_len ==0 ) p_uart->dma_rx_len  = UART_RX_DMA_SIZE;
			p_uart->rx_dma_ch = ks_driver_dma_bind_channel(handle);
			if(p_uart->rx_dma_ch == -1) ks_exception_assert(0);
			_uart_dma_recieve_init(p_uart,p_uart->dma_rx_len);
		}
	}

	if(uart_task_created == 0)
	{
		ks_os_flag_create(&flag_handle_uart_tx, "__uart_tx");
		ks_os_flag_create(&flag_handle_uart_rx, "__uart_rx");

		ks_os_thread_create(&thread_handle_uart_print,			//	thread_handle
						 "uart_print_task",	//	thread_name
						 _uart_print_task,					//	thread_entry
						 0,									//	arg
						 _THREAD_PRI_UART_PRINT,			//	priority
						 thread_stack_uart_print,			//	stack_start
						 sizeof(thread_stack_uart_print),	//	stack_size
						 0,
						 1
						 );

		ks_os_thread_create(&thread_handle_uart_entry,			//	thread_handle
						 "uart_entry_task",	//	thread_name
						 _uart_entry_task,					//	thread_entry
						 0,									//	arg
						 _THREAD_PRI_UART_RECEIVE,			//	priority
						 thread_stack_uart_entry,			//	stack_start
						 sizeof(thread_stack_uart_entry),	//	stack_size
						 0,
						 1
						 );


		ks_shell_add_cmds(uart_driver_cmds, sizeof(uart_driver_cmds) / sizeof(cmd_proc_t));
		uart_task_created = 1;
	
	}

	return 0;
}



int _uart_dma_send(pUartDriver p_uart,	U8 * dmabuffer,int len,int dma_ch){
	int ret;
	int ch = -1;

	int level = 1;

	OSHandle dma_handle = ks_driver_dma_get_handle();
	ch= ks_driver_dma_alloc_channel(dma_handle, dma_ch);
	if(ch >= 0){
		ret =_uart_low_level_config_tx_dma_channel(dma_handle,&p_uart->hUART,ch,(dma_event_cb)_uart_dma_event_cb,(void*) p_uart);
		if(ret < 0) {
			ks_driver_dma_release_channel(dma_handle,ch);
			return -1;
		}

		ret =_uart_low_level_start_tx_dma(dma_handle,&p_uart->hUART,ch, dmabuffer, len);
		if(ret != 0) {
			ks_driver_dma_release_channel(dma_handle,ch);
			return -2;
		}
		
		return 0;
	}

	return -3;

}

int _uart_dma_recieve(pUartDriver p_uart,	U8 * dmabuffer,int len,int dma_ch){
	int ret;
	int ch = -1;

	int level = 1;

	OSHandle dma_handle = ks_driver_dma_get_handle();
	ch= ks_driver_dma_alloc_channel(dma_handle, dma_ch);
	if(ch >= 0){
		ret =_uart_low_level_config_rx_dma_channel(dma_handle,&p_uart->hUART,ch,(dma_event_cb)_uart_dma_event_cb,(void*) p_uart);
		if(ret < 0) {
			ks_driver_dma_release_channel(dma_handle,ch);
			return -1;
		}

		ret =_uart_low_level_start_rx_dma(dma_handle,&p_uart->hUART,ch, dmabuffer, len);
		if(ret != 0) {
			ks_driver_dma_release_channel(dma_handle,ch);
			return -2;
		}
		
		return 0;
	}

	return -3;

}
int _uart_fifo_send(pUartDriver p_uart,	U8 * fifobuffer,int len){

	p_uart->hUART.tx_fifo_count += len;
	_uart_low_level_write_tx_fifo(&p_uart->hUART, (char*)fifobuffer, len);
	return 0;
}


int _uart_dma_recieve_init(pUartDriver p_uart,int dma_len)
{
	int len = 0;

	len= ringbuffer_get_dma_buffer_widx(&p_uart->rx_ringbuffer,&p_uart->dma_buffer_rx,dma_len);
	
	//ks_os_printf(0,"dmarx_start dma_buffer_rx %x	len %d dma_len %d  \r\n",p_uart->dma_buffer_rx,len,dma_len);
	ks_cpu_dcache_invalidate(p_uart->dma_buffer_rx,len);
	
	p_uart->hUART.dma_rx_status = 1;
	int ret =  _uart_dma_recieve(p_uart, p_uart->dma_buffer_rx,dma_len,p_uart->rx_dma_ch);
	if(ret==0){

	}else{
		p_uart->hUART.dma_rx_status = 0;
	}

	return  ret;
}




int _uart_dma_recieve_restart(pUartDriver p_uart,int dma_len)
{
	int len = 0;
	
	OSHandle dma_handle  = ks_driver_dma_get_handle();

	len = ringbuffer_get_dma_buffer_widx(&p_uart->rx_ringbuffer,&p_uart->dma_buffer_rx,dma_len);
	//ring buffer满,lost data
	if(len == 0){
		//ks_os_printf(0,"_uart_dma_recieve_restart ringfull  dma_len %d	len %d widx %d  ridx %d ++\r\n",dma_len,len,p_uart->rx_ringbuffer.widx,p_uart->rx_ringbuffer.ridx);
		p_uart->dma_buffer_rx = (U8*) (((U32)g_dma_rx_buffer) + p_uart->hUART.uart_id * UART_RX_DMA_SIZE);
		len = dma_len;
		p_uart->hUART.rx_dma_lost_count += dma_len;
	}


	//ks_os_printf(0,"_uart_dma_recieve_restart dma_buffer_rx %x len %d dma_len %d  \r\n",p_uart->dma_buffer_rx,len,dma_len);
	ks_cpu_dcache_invalidate(p_uart->dma_buffer_rx,len);

	_uart_low_level_config_rx_int(&p_uart->hUART,1);
	//_uart_low_level_config_rx_int(&p_uart->hUART,0);
	//_uart_low_level_config_rx_fifo_char_1(&p_uart->hUART);
	_uart_low_level_config_rx_fifo_half_full(&p_uart->hUART);

	p_uart->hUART.dma_rx_status = 1;

	int ret =  _uart_low_level_start_rx_dma(dma_handle,&p_uart->hUART,p_uart->rx_dma_ch,p_uart->dma_buffer_rx, len);
	
	if(ret==0){

	}else{
		p_uart->hUART.dma_rx_status = 0;
	}

	return  ret;
}


int _uart_dma_read_fifo(pUartDriver p_uart,int dma_len)
{
	int len = 0;
	
	OSHandle dma_handle  = ks_driver_dma_get_handle();


	len = ringbuffer_get_dma_buffer_widx(&p_uart->rx_ringbuffer,&p_uart->dma_buffer_rx,dma_len);
	//ring buffer满,lost data
	if(len == 0){
		//ks_os_printf(0,"_uart_dma_read_fifo ringfull  dma_len %d	len %d widx %d  ridx %d ++\r\n",dma_len,len,p_uart->rx_ringbuffer.widx,p_uart->rx_ringbuffer.ridx);
		p_uart->dma_buffer_rx = (U8*) (((U32)g_dma_rx_buffer) + p_uart->hUART.uart_id * UART_RX_DMA_SIZE);
		len = dma_len;
		p_uart->hUART.rx_dma_lost_count += dma_len;
	}
	

	//ks_os_printf(0,"_uart_dma_read_fifo dma_buffer_rx %x len %d dma_len %d  \r\n",p_uart->dma_buffer_rx,len,dma_len);
	ks_cpu_dcache_invalidate(p_uart->dma_buffer_rx,dma_len);

	//dma 长度小与FIFO 长度，配置FIFO  触发阀值，才能读出
	_uart_low_level_config_rx_int(&p_uart->hUART,0);
	_uart_low_level_config_rx_fifo_char_1(&p_uart->hUART);

	p_uart->hUART.dma_rx_status = 1;

	int ret =  _uart_low_level_start_rx_dma(dma_handle,&p_uart->hUART,p_uart->rx_dma_ch,p_uart->dma_buffer_rx, len);
	
	if(ret==0){

	}else{
		p_uart->hUART.dma_rx_status = 0;
	}

	return  ret;
}

void _uart_print_task(void *p_arg)
{
	ks_os_thread_vfp_enable();
	int ret ,ch;
	U8 * fifobuffer;
	U8 * dmabuffer;
	int dmalen ;
	int len;
	U32 uart_all_tx_flags = 0;
	U32 channel_flag;
	
	for (int i=0; i<UART_NUM; i++)
		uart_all_tx_flags |= (1 << i);
	
	while(1)
	{
		U32 curFlag = ks_os_flag_wait(flag_handle_uart_tx, uart_all_tx_flags, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
		while (curFlag != 0)
		{
			int uartID = -1;
			for (int i=0; i<UART_NUM; i++)
			{
				if (curFlag & (1 << i))
				{
					uartID = i;
					curFlag &= ~(1 << i);
					break;
				}
			}
			if (uartID == -1)
				break;
			

			pUartDriver p_uart = &g_uart[uartID];
			len = _get_tx_buffer_length(p_uart);
				
			if(p_uart->tx_dma_enable){
				
				//dma transing  contine;
				if(p_uart->hUART.dma_tx_status == 1){
					continue;
				}
				
				dmalen = _get_tx_dma_buffer_len(p_uart,len);
				if (dmalen > 0){
					dmalen = _get_tx_dma_buffer(p_uart,&dmabuffer,dmalen);
					ks_cpu_dcache_clean(dmabuffer,dmalen);
					p_uart->hUART.dma_tx_status = 1;
					p_uart->hUART.tx_dma_req_count += dmalen;
					ret = _uart_dma_send(p_uart,dmabuffer,dmalen,p_uart->tx_dma_ch);
					if(ret==0){
						//dma start ok continue next loop
						_update_tx_buffer_offset(p_uart,dmalen);
						continue;
					}
					else if(ret == -3){ 
						//alloc channel failure go to fifi trans
						p_uart->hUART.dma_tx_status = 0;
						p_uart->hUART.dma_failure++;
					}
					else{
						// when dma channel get no free start dma erro, fifo send 
						p_uart->hUART.dma_tx_status = 0;
						p_uart->hUART.dma_erro++;
					}
				}
			}else{
				_uart_low_level_config_tx_int(&p_uart->hUART, 0);
				unsigned int fifoSize, fifoUsed;
				fifoSize = _uart_low_level_get_fifo_size(&p_uart->hUART);
				fifoUsed = _uart_low_level_get_tx_fifo_level(&p_uart->hUART);
				int sizeAval = fifoSize - fifoUsed;
				if (len != 0)
				{
					if (len > sizeAval)
						len = sizeAval;
					
					int fifolen = _get_tx_fifo_buffer(p_uart,&fifobuffer,len);
					_uart_fifo_send(p_uart,fifobuffer,fifolen);
					_update_tx_buffer_offset(p_uart,fifolen);
				}
				len = _get_tx_buffer_length(p_uart);
				if (len > 0)
					_uart_low_level_config_tx_int(&p_uart->hUART, 1);
			}
		}
	}
}




void _uart_isr_handler(void* arg )
{
	UARTIntType intStatus;
	S32 fifo_level;
	int i;
	int uart_id = (int )arg;
	int irq_vec_num = _get_uart_irq_vector(UART_START_INDEX[uart_id]);
	ks_os_irq_disable(irq_vec_num);
	char temp[128];

	pUartDriver p_uart = &g_uart[uart_id];

	intStatus = _uart_low_level_get_int_type(&p_uart->hUART);
	
	if (intStatus == UART_INT_TX_EMPTY)
	{
		//	发送缓存空，触发发送信号
		ks_os_flag_set_bits(flag_handle_uart_tx, (1 << uart_id), SET_FLAG_MODE_OR);
	}
	else if ((intStatus == UART_INT_RX_VALID) || (intStatus == UART_INT_RX_TIMEOUT))
	{
		if((intStatus == UART_INT_RX_TIMEOUT) && (p_uart->hUART.dma_rx_status!=0)) {
		//if((p_uart->hUART.dma_rx_status!=0)) {
			OSHandle dma_handle  = ks_driver_dma_get_handle();
			U32 src_offset,dst_offset = 0;
			
			ks_driver_dma_suspend(dma_handle,p_uart->rx_dma_ch,1);
			
			ks_driver_dma_get_trans_offset(dma_handle,p_uart->rx_dma_ch,&src_offset,&dst_offset);
			if (dst_offset > 0){	
				p_uart->hUART.rx_dma_br_count+= dst_offset;
				p_uart->rx_count += dst_offset;
				//ringbuffer_write(&p_uart->rx_ringbuffer,p_uart->dma_buffer_rx, dst_offset);
				//ks_os_printf_hex(0,p_uart->dma_buffer_rx,dst_offset);
				ringbuffer_update_widx(&p_uart->rx_ringbuffer,dst_offset);
				ks_cpu_dcache_invalidate(p_uart->dma_buffer_rx,dst_offset);
				//ks_os_printf(0,"1 dma_buffer_rx %x len %d \r\n",p_uart->dma_buffer_rx,dst_offset);
			}
			
	
			ks_driver_dma_stop(dma_handle,p_uart->rx_dma_ch);
			//清空suspend 标志
			ks_driver_dma_suspend(dma_handle,p_uart->rx_dma_ch,0);
			
			p_uart->hUART.dma_rx_status = 0;

			fifo_level = _uart_low_level_get_rx_fifo_level(&p_uart->hUART);
			//ks_os_printf(0,"_uart_isr_handler timerout dma %d fifo %d \r\n",dst_offset,fifo_level);

			if(fifo_level!=0){
				_uart_dma_read_fifo(p_uart,fifo_level);
			}else{
				_uart_dma_recieve_restart(p_uart,p_uart->dma_rx_len);		
			}
			
			ks_os_flag_set_bits(flag_handle_uart_rx, (1 << uart_id), SET_FLAG_MODE_OR);

			ks_os_irq_enable(irq_vec_num);

			return ;

		}

read_fifo:
		fifo_level = _uart_low_level_get_rx_fifo_level(&p_uart->hUART);
		//ks_os_printf(0,"_uart_isr_handler intStatus %d fifo %d \r\n",intStatus,fifo_level);

		if (fifo_level != 0){
			U8*	p_bufferout;
			//fifo 直接操作ringbuffer ,同样需要处理ringbuffer 边界问题
			int len = ringbuffer_get_dma_buffer_widx(&p_uart->rx_ringbuffer,&p_bufferout,fifo_level);
			if(len > 0){
				_uart_low_level_read_rx_fifo(&p_uart->hUART, (char*)p_bufferout, len);
				ringbuffer_update_widx(&p_uart->rx_ringbuffer,len);
				p_uart->hUART.rx_fifo_count+= len;
				p_uart->rx_count += len;
				//到达 ringbuffer 边界，跳过边界重新读取
				if(len < fifo_level){
					goto read_fifo;
				}
			}else{//ringbuffer满
				_uart_low_level_read_rx_fifo(&p_uart->hUART, temp, fifo_level);
				p_uart->hUART.rx_fifo_lost+= fifo_level;	
			}
		}

		ks_os_flag_set_bits(flag_handle_uart_rx, (1 << uart_id), SET_FLAG_MODE_OR);
		
	}
	ks_os_irq_enable(irq_vec_num);
}


void _uart_dma_event_cb(dma_channel_context* ch_ctx,int32_t ch, dma_event_e event){

	pUartDriver p_uart = ch_ctx->arg;
	pUARTHandle uart = &p_uart->hUART;

	OSHandle dma_handle  = ks_driver_dma_get_handle();

	
	if (event == DMA_EVENT_TRANSFER_ERROR) {
		uart->dma_erro++;
	}else if(event == DMA_EVENT_TRANSFER_DONE){
		if((uart->dma_tx_status != 0) && (uart->dma_tx_channel == (U32)ch)){
			uart->tx_dma_ok_count += ch_ctx->len ;			
			uart->dma_tx_status = 0;
			ks_driver_dma_stop(dma_handle,ch);
			ks_driver_dma_release_channel(dma_handle,ch);
			ks_os_flag_set_bits(flag_handle_uart_tx, (1 << uart->uart_id), SET_FLAG_MODE_OR);
		}else if((uart->dma_rx_status!=0) && (uart->dma_rx_channel == (U32)ch)){
			uart->dma_rx_status = 0;
			ks_driver_dma_stop(dma_handle,ch);
			//ringbuffer_write(&p_uart->rx_ringbuffer,p_uart->dma_buffer_rx,ch_ctx->len);
			//判断  dma 接收为 ringbuffer 有效范围内
			if(ringbuffer_check_inrange(&p_uart->rx_ringbuffer,p_uart->dma_buffer_rx)){
				ringbuffer_update_widx(&p_uart->rx_ringbuffer,ch_ctx->len);
				ks_cpu_dcache_invalidate(p_uart->dma_buffer_rx,ch_ctx->len);
				//ks_os_printf(0,"2 dma_buffer_rx %x len %d \r\n", p_uart->dma_buffer_rx,ch_ctx->len);
				p_uart->hUART.rx_dma_ok_count += ch_ctx->len;
			}
			
			p_uart->rx_count += ch_ctx->len;
			ks_os_flag_set_bits(flag_handle_uart_rx, (1 << uart->uart_id), SET_FLAG_MODE_OR);

			int fifo_level = _uart_low_level_get_rx_fifo_level(&p_uart->hUART);
		
			if(fifo_level!=0){
				_uart_dma_read_fifo(p_uart,fifo_level);
			}else{
				_uart_dma_recieve_restart(p_uart,p_uart->dma_rx_len);		
			}

		}else{
			ks_exception_assert(0);
		}
	}

}

S32 _uart_get_baudrate(int uart_id)
{
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return -1;
	pUartDriver p_uart = &g_uart[uart_id];
	if (p_uart->enable == 0)
		return -1;
	return p_uart->hUART.uart_baud;
}

S32 _uart_get_stopbit(int uart_id)
{
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return -1;
	pUartDriver p_uart = &g_uart[uart_id];
	if (p_uart->enable == 0)
		return -1;
	return p_uart->hUART.stop_bits;
}

S32 _uart_get_datawidth(int uart_id)
{
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return -1;
	pUartDriver p_uart = &g_uart[uart_id];
	if (p_uart->enable == 0)
		return -1;
	return p_uart->hUART.data_width;
}

S32 _uart_get_parity(int uart_id)
{
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return -1;
	pUartDriver p_uart = &g_uart[uart_id];
	if (p_uart->enable == 0)
		return -1;
	return p_uart->hUART.parity;
}

S32 _uart_send_buffer(int uart_id, const U8 *p_data, int len)
{
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return DRV_ERROR_PARAMETER ;
	if (len <= 0)
		return DRV_ERROR_PARAMETER;

	pUartDriver p_uart = &g_uart[uart_id];
	if (p_uart->enable == 0)
		return DRV_ERROR_STATUS;
	if (ks_bb_get_debug_port() == uart_id)
		return DRV_ERROR_STATUS;  // Skip if current port is set as BP debug port

	int tx_len = _en_tx_buffer(p_uart, p_data, len);

	ks_os_flag_set_bits(flag_handle_uart_tx, (1 << uart_id), SET_FLAG_MODE_OR);

	return tx_len;
}


int  _uart_send_string(int uart_id, const char send_string[])
{
	int len = strlen(send_string);
	return _uart_send_buffer(uart_id, (const U8*)send_string, len);
}

int _uart_read_byte_from_any(U8 *pData)
{
	int dmalen = 1;
	uint8_t* dmabuffer;

	U32 rxAllFlags = 0;
	//如果RX buffer中已经有数据，则直接取出
	for (int i=0; i<UART_NUM; i++)
	{
		rxAllFlags |= (1 << i);
		pUartDriver p_uart = &g_uart[i];

		int avalByte =ringbuffer_available_read_space(&p_uart->rx_ringbuffer);
		if (avalByte != 0)
		{
			/*if(p_uart->rx_dma_enable){
				//uint32_t addr = 0xC0000000 + (uint32_t)&p_uart->rx_ringbuffer.buffer[p_uart->rx_ringbuffer.ridx];
				dmalen = ringbuffer_get_dma_buffer_ridx(&p_uart->rx_ringbuffer,&dmabuffer,dmalen);
				ks_cpu_dcache_invalidate(dmabuffer,dmalen);
				//ks_os_printf(0,"dmabuffer %x\r\n",dmabuffer);
			}*/
			ringbuffer_read(&p_uart->rx_ringbuffer,pData,1);
			
			return i;
		}
	}
	//所有串口的RX buffer均为空的情况下，等待任何一个串口收到数据
	while (1)
	{
		U32 curFlag = ks_os_flag_wait(flag_handle_uart_rx, rxAllFlags, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
		int uartID = -1;
		for (int i=0; i<UART_NUM; i++)
		{
			if (curFlag & (1 << i))
			{
				uartID = i;
				break;
			}
		}
		if (uartID == -1)
			continue;
		
		pUartDriver p_uart = &g_uart[uartID];

		int avalByte =ringbuffer_available_read_space(&p_uart->rx_ringbuffer);
		if (avalByte != 0)
		{
			/*if(p_uart->rx_dma_enable){
				//uint32_t addr = 0xC0000000 + (uint32_t)&p_uart->rx_ringbuffer.buffer[p_uart->rx_ringbuffer.ridx];
				dmalen = ringbuffer_get_dma_buffer_ridx(&p_uart->rx_ringbuffer,&dmabuffer,dmalen);
				ks_cpu_dcache_invalidate(dmabuffer,dmalen);
				
				ks_os_printf(0,"dmabuffer %x\r\n",dmabuffer);
				//uint32_t addr = &p_uart->rx_ringbuffer.buffer[p_uart->rx_ringbuffer.ridx] ;
			}*/
			ringbuffer_read(&p_uart->rx_ringbuffer,pData,1);

			return uartID;
		}
	}
}



void _de_tx_buffer(pUartDriver p_uart, U8* p_buffer, S32 len)
{
	ks_os_mutex_enter(p_uart->mux);
	if (len > p_uart->tx_valid)
		len = p_uart->tx_valid;
	S32 t = p_uart->tx_read + len;
	if (t > UART_BUF_TX_SIZE)
	{
		S32 temp = UART_BUF_TX_SIZE - p_uart->tx_read;
		memcpy(p_buffer, &p_uart->tx_buf[p_uart->tx_read], temp);
		memcpy(p_buffer + temp, p_uart->tx_buf, len - temp);
		p_uart->tx_read = len - temp;
	}
	else
	{
		memcpy(p_buffer, &p_uart->tx_buf[p_uart->tx_read], len);
		p_uart->tx_read += len;
	}
	p_uart->tx_valid -= len;
	ks_os_mutex_leave(p_uart->mux);
}




#if 0
int _get_tx_buffer_length(pUartDriver p_uart)
{
	int temp = 0;

	temp = p_uart->tx_rear - p_uart->tx_front;

	return (temp >= 0 ? temp : temp + UART_BUF_TX_SIZE);
}

void _en_tx_buffer(pUartDriver p_uart, U8 info)
{
	p_uart->tx_buf[p_uart->tx_rear] = info;
	p_uart->tx_rear = (p_uart->tx_rear >= UART_BUF_TX_SIZE - 1) ? 0 : p_uart->tx_rear + 1;
	if (p_uart->tx_rear == p_uart->tx_front)
	{
		p_uart->enable = p_uart->tx_rear;
		p_uart->enable *= p_uart->tx_front;
	}
}

U8 _de_tx_buffer(pUartDriver p_uart)
{
	U8 temp = 0;
	temp = p_uart->tx_buf[p_uart->tx_front];
	p_uart->tx_front = (p_uart->tx_front >= UART_BUF_TX_SIZE - 1) ? 0 : p_uart->tx_front + 1;
	return(temp);
}
#else
int _get_tx_buffer_length(pUartDriver p_uart)
{
	return p_uart->tx_valid;
}

int _get_tx_buffer_length_for_uart(int uart_id)
{
	pUartDriver p_uart = &g_uart[uart_id];
	if (p_uart->enable == 0)
		return 0;
	return p_uart->tx_valid;
}

int _en_tx_buffer(pUartDriver p_uart, const U8* p_data, S32 len)
{
	ks_os_mutex_enter(p_uart->mux);
	S32 residual = UART_BUF_TX_SIZE - p_uart->tx_valid;
	if (len > residual){
		p_uart->tx_lost = len - residual;
		len = residual;
	}

	S32 t = p_uart->tx_write + len;
	if (t > UART_BUF_TX_SIZE)
	{
		S32 temp = UART_BUF_TX_SIZE - p_uart->tx_write;
		memcpy(&p_uart->tx_buf[p_uart->tx_write], p_data, temp);
		memcpy(p_uart->tx_buf, p_data + temp, len - temp);
		p_uart->tx_write = len - temp;
	}
	else
	{
		memcpy(&p_uart->tx_buf[p_uart->tx_write], p_data, len);
		p_uart->tx_write += len;
	}
	p_uart->tx_valid += len;
	p_uart->tx_count += len;
	ks_os_mutex_leave(p_uart->mux);
	return len;
}



int _get_tx_dma_buffer_len(pUartDriver p_uart, S32 len){

	if (len > p_uart->tx_valid){
		len = p_uart->tx_valid;
	}
	
	if (len > UART_TX_DMA_SIZE){
		len = (UART_TX_DMA_SIZE);
	}

	
	S32 t = p_uart->tx_read + len;
	if (t > UART_BUF_TX_SIZE)
	{
		len = UART_BUF_TX_SIZE - p_uart->tx_read;
	}
	else
	{

	}
	return len;
}


int _get_tx_fifo_buffer(pUartDriver p_uart, U8** p_bufferout, S32 len)
{

	int fifolen;

	if (len > p_uart->tx_valid)
		len = p_uart->tx_valid;
	

	S32 t = p_uart->tx_read + len;
	if (t > UART_BUF_TX_SIZE)
	{
		fifolen = UART_BUF_TX_SIZE - p_uart->tx_read;
		*p_bufferout = &p_uart->tx_buf[p_uart->tx_read];
	}
	else
	{
		*p_bufferout = &p_uart->tx_buf[p_uart->tx_read];
		fifolen = len;
	}


	return fifolen;


}


int _get_tx_dma_buffer(pUartDriver p_uart, U8** p_bufferout, S32 len)
{

	int dmalen;

	if (len > p_uart->tx_valid)
		len = p_uart->tx_valid;
	

	S32 t = p_uart->tx_read + len;
	if (t > UART_BUF_TX_SIZE)
	{
		dmalen = UART_BUF_TX_SIZE - p_uart->tx_read;
		*p_bufferout = &p_uart->tx_buf[p_uart->tx_read];
	}
	else
	{
		*p_bufferout = &p_uart->tx_buf[p_uart->tx_read];
		dmalen = len;
	}


	return dmalen;
}

void _update_tx_buffer_offset(pUartDriver p_uart,  S32 len)
{

	ks_os_mutex_enter(p_uart->mux);

	if (len > p_uart->tx_valid)
		len = p_uart->tx_valid;
	
	S32 t = p_uart->tx_read + len;
	if (t > UART_BUF_TX_SIZE)
	{
		S32 temp = UART_BUF_TX_SIZE - p_uart->tx_read;
		p_uart->tx_read = len - temp;
		
		ks_exception_assert(0);
		
	}else if(t == UART_BUF_TX_SIZE)
	{
		p_uart->tx_read  = 0;
	}
	else
	{
		p_uart->tx_read += len;
	}
	p_uart->tx_valid -= len;
	
	ks_os_mutex_leave(p_uart->mux);

}



#endif
int _uart_register_protocol(int uart_id, UartEntry p_parse_input)
{
	int i;
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return 1;

	if (g_handle_serial_port[uart_id] == 0)
		return 2;

	for (i = 0; i < MAX_PROTOCOL_NUMBER; i++)
	{
		if (g_uart_parser[uart_id][i] == p_parse_input)
		{
			return 3;
		}
	}
	for (i = 0; i < MAX_PROTOCOL_NUMBER; i++)
	{
		if (g_uart_parser[uart_id][i] == 0)
		{
			g_uart_parser[uart_id][i] = p_parse_input;
			break;
		}
	}
	if (i == 3)
		return 4;
	return 0;
}

int _uart_deregister_protocol(int uart_id, UartEntry p_parse_input)
{
	int i;
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return 1;
	if (g_handle_serial_port[uart_id] == 0)
		return 2;
	for (i = 0; i < MAX_PROTOCOL_NUMBER; i++)
	{
		if (g_uart_parser[uart_id][i] == p_parse_input)
			g_uart_parser[uart_id][i] = 0;
	}
	return 0;
}


void _uart_entry_task(void *p_arg)
{
	ks_os_thread_vfp_enable();

	int i, result;

	while (1)
	{
		U8 cur_byte;
		
		int uart_id = _uart_read_byte_from_any(&cur_byte);  

		result = 0;
		for (i = 0; i < MAX_PROTOCOL_NUMBER; i++)
		{
			if (g_uart_parser[uart_id][i] != 0)
			{
				result |= (*g_uart_parser[uart_id][i])(uart_id, cur_byte);
			}
		}

		if (result)
		{
			for (i = 0; i < MAX_PROTOCOL_NUMBER; i++)
			{
				if (g_uart_parser[uart_id][i] != 0)
				{
					(*g_uart_parser[uart_id][i])(-1, uart_id);
				}
			}
		}
	}
}




S32 _uart_config(int uart_id, int baud, int data_width, int parity, int stop_bits)
{
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return DRV_ERROR_PARAMETER ;


	ks_exception_assert(data_width >= UART_DATA_BITS_5 && data_width <= UART_DATA_BITS_8);
		
	ks_exception_assert(stop_bits >= UART_STOP_BITS_1 && stop_bits <= UART_STOP_BITS_1_5);
	

	pUartDriver p_uart = &g_uart[uart_id];
	p_uart->enable = 0;
	int fifo_used = _uart_low_level_get_tx_fifo_level(&p_uart->hUART);

	while(fifo_used != 0)
	{
		ks_os_thread_sleep(1);
		fifo_used = _uart_low_level_get_tx_fifo_level(&p_uart->hUART);
	}

	//1.5 stop bits when DLS(LCR[1:0]) is zero, else 2 stop bit
	if (data_width == UART_DATA_BITS_5)
	{
	  ks_exception_assert(stop_bits != UART_STOP_BITS_2);
	}
	else
	{
	  ks_exception_assert(stop_bits != UART_STOP_BITS_1_5);
	}

	int irq_vec_num = _get_uart_irq_vector(UART_START_INDEX[uart_id]);
	
	ks_os_irq_disable(irq_vec_num);
	
	_uart_low_level_set_baud_rate(&p_uart->hUART, baud);
	
	_uart_low_level_set_parity(&p_uart->hUART,parity);

	_uart_low_level_set_stop_bits(&p_uart->hUART,stop_bits);
	
	_uart_low_level_set_data_bits(&p_uart->hUART,data_width);
	
	_uart_low_level_setup_fifo(&p_uart->hUART, 1);
	
	ks_os_irq_enable(irq_vec_num);
	
	_uart_low_level_config_rx_int(&p_uart->hUART, 1);


	p_uart->enable = 1;

	return 0;
}


//初始化串口，不使能中断　用于平台启动时候(gic 没有初始化)　调试
void _uart_poll_init(int uart_id, int baud,int sel)
{
	UART_START_INDEX[uart_id] = uart_id;
	
	if(uart_id > 3 ) return ;
		
		
	if(uart_id == 1)
	{
		ks_driver_sysctrl_set_mux_sel(GPIOA_17,MUX_V_FUN1);
		ks_driver_sysctrl_set_mux_sel(GPIOA_18,MUX_V_FUN1);
	}
	else if(uart_id == 2)
	{
		ks_driver_sysctrl_set_mux_sel(GPIOA_19,MUX_V_FUN1);
		ks_driver_sysctrl_set_mux_sel(GPIOA_20,MUX_V_FUN1);

	}
	else if(uart_id == 3)
	{
		if(sel==0){
			ks_driver_sysctrl_set_mux_sel(GPIOA_12,MUX_V_FUN2);
			ks_driver_sysctrl_set_mux_sel(GPIOA_13,MUX_V_FUN2);
		}else{
			ks_driver_sysctrl_set_mux_sel(GPIOA_4,MUX_V_FUN3);
			ks_driver_sysctrl_set_mux_sel(GPIOA_5,MUX_V_FUN3);
		}
	}
	else
	{

	}
	pUartDriver p_uart = &g_uart[uart_id];

	memset(p_uart, 0, sizeof(UartDriver));

	p_uart->enable = 1;

	_uart_low_level_init(UART_START_INDEX[uart_id], &p_uart->hUART);
	//_uart_low_level_set_baud_rate(&p_uart->hUART, baud);
	_uart_low_level_setup_fifo(&p_uart->hUART, 1);

	//int irq_vec_num = _get_uart_irq_vector(UART_START_INDEX[uart_id]);

	//ks_os_irq_create(irq_vec_num, _uart_isr_handler, uart_id);
	//ks_os_irq_enable(irq_vec_num);
	//ks_os_irq_map_target(irq_vec_num, 1);

	_uart_low_level_config_rx_int(&p_uart->hUART, 1);

}

void _uart_enter_poll_mode(int uart_id)
{
	int irq_vec_num = _get_uart_irq_vector(UART_START_INDEX[uart_id]);
	ks_os_irq_disable(irq_vec_num);
}

void _uart_poll_send_buffer(int uart_id, char *p_data, int len)
{
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return ;

	pUartDriver p_uart = &g_uart[uart_id];
	_uart_low_level_print(&p_uart->hUART, p_data, len);
}

void _uart_poll_send_byte(int uart_id, char data)
{
	if ((uart_id < 0) || (uart_id > (UART_NUM-1)))
		return ;

	pUartDriver p_uart = &g_uart[uart_id];
	_uart_low_level_print(&p_uart->hUART, &data, 1);
}

void _uart_poll_send_string(int uart_id, char send_string[])
{
	int len = strlen(send_string);
	_uart_poll_send_buffer(uart_id, send_string, len);
}

char _uart_poll_rcv_byte(int uart_id)
{
	pUartDriver p_uart = &g_uart[uart_id];
	return _uart_low_level_get_byte(&p_uart->hUART);
}
