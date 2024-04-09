
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  uart.c
 *
 * @brief  uart 演示实现　
 */


#include <stdlib.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "shell_api.h"
#include "ks_uart.h"




#define  STACK_SIZE_UART_TASK          512
static S32 thread_stack_uart_task[STACK_SIZE_UART_TASK];
static OSHandle flag_handle_uart_task;
static OSHandle thread_handle_uart_task;

static OSHandle timer_uart ;


#define  STACK_SIZE_TEST_TASK          1024
static S32 thread_stack_dmatx_task[STACK_SIZE_TEST_TASK];
static OSHandle flag_handle_dmatx_task;
static OSHandle thread_handle_dmatx_task;




static S32 thread_stack_tx_loop_task[STACK_SIZE_UART_TASK];
static OSHandle flag_handle_tx_loop_task;
static OSHandle thread_handle_tx_loop_task;




static S32 thread_stack_rx_loop_task[STACK_SIZE_UART_TASK];
static OSHandle flag_handle_rx_loop_task;
static OSHandle thread_handle_rx_loop_task;



static S32 thread_stack_txrx_loop_task[STACK_SIZE_UART_TASK];
static OSHandle flag_handle_txrx_loop_task;
static OSHandle thread_handle_txrx_loop_task;


static uint32_t  s_tx_uartid ;
static uint32_t  s_tx_len ;
static uint32_t  s_tx_loop_enable;

static uint32_t  s_rx_uartid ;
static uint32_t  s_rx_len ;
static uint32_t  s_rx_loop_ready ;
static uint32_t  s_rx_loop_enable;

static uint32_t  s_tx_enbale;

static uint32_t  s_tx_total_count ;
static int  txrx_len;

static char* s_tx_str = "#123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890123456789012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890012345678901234567890^";
static OSHandle timer_loop;


static U8 g_tx_buffer[2048] __attribute__ ((aligned (64)));
static U8 g_rx_buffer[2048] __attribute__ ((aligned (64)));

#define TXRX_LOOP_LEN 1024
/**
 * 设置uart 波特率命令实现　
 * @param ctx [in]
 *      命令上下文指针
 * @param argc [in]
 *      输入参数个数
  * @param argc [in]
 *      输入参数组指针
 * @return　
 *      
 */
static int uart_echo_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int i, iret;
	uint32_t uart_id;
    uint32_t baud;
    uint32_t len;

    len = 0;

    if (argc > 2 )
    {
		iret = ks_shell_str2uint(argv[1], &uart_id);
		 if (iret != 0)
		 {
			 return CMD_ERR_PARAMS_FORMAT;
		 }

		 
		 ks_driver_uart_send_string(uart_id, argv[2]);
    }
    else
    {
        return CMD_ERR_PARAMS_NOT_ENOUGH;
    }

    return 0;
}




/**
 * 设置uart 波特率命令实现　
 * @param ctx [in]
 *      命令上下文指针
 * @param argc [in]
 *      输入参数个数
  * @param argc [in]
 *      输入参数组指针
 * @return　
 *      
 */
static int uart_baud_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int i, iret;
	uint32_t uart_id;
    uint32_t baud;
    uint32_t len;

    len = 0;

    if (argc > 2 )
    {
		iret = ks_shell_str2uint(argv[1], &uart_id);
		 if (iret != 0)
		 {
			 return CMD_ERR_PARAMS_FORMAT;
		 }
		 iret = ks_shell_str2uint(argv[2], &baud);
		 if (iret != 0)
		 {
			 return CMD_ERR_PARAMS_FORMAT;
		 }
		 
		 ks_driver_uart_config(uart_id, baud);
		 
		 ks_shell_printf(ctx->uart_id, "uart%d baud has set %d !!!\r\n",uart_id, baud);
    }
    else
    {
        return CMD_ERR_PARAMS_NOT_ENOUGH;
    }

    return 0;
}




/**
 * 设置uart 波特率命令实现　
 * @param ctx [in]
 *      命令上下文指针
 * @param argc [in]
 *      输入参数个数
  * @param argc [in]
 *      输入参数组指针
 * @return　
 *      
 */
static int uart_config_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int i, iret;
	uint32_t uart_id;
    uint32_t baud;
    uint32_t len;
    uint32_t data_width;
    uint32_t parity;
    uint32_t stop_bits;

    len = 0;

    if (argc >= 2 )
    {
    	
		 iret = ks_shell_str2uint(argv[1], &uart_id);
		 if (iret != 0)
		 {
			 return CMD_ERR_PARAMS_FORMAT;
		 }

		 if(argc  == 2 ){
			 ks_shell_printf(ctx->uart_id, "uart%d	config baud %d data_width %d parity %d stop_bits %d !!!\r\n",uart_id, ks_driver_uart_get_baud(uart_id),ks_driver_uart_get_datawidth(uart_id),ks_driver_uart_get_parity(uart_id),ks_driver_uart_get_stopbit(uart_id));
		 	 return CMD_ERR_OK;

		 }
	
		 iret = ks_shell_str2uint(argv[2], &baud);
		 if (iret != 0)
		 {
			 return CMD_ERR_PARAMS_FORMAT;
		 }
	
		 if(argc  == 3 ){
		 	 ks_driver_uart_config_ex(uart_id, baud,ks_driver_uart_get_datawidth(uart_id),ks_driver_uart_get_parity(uart_id),ks_driver_uart_get_stopbit(uart_id));
			 ks_shell_printf(ctx->uart_id, "uart%d	config baud %d data_width %d parity %d stop_bits %d !!!\r\n",uart_id, ks_driver_uart_get_baud(uart_id),ks_driver_uart_get_datawidth(uart_id),ks_driver_uart_get_parity(uart_id),ks_driver_uart_get_stopbit(uart_id));
		 	 return CMD_ERR_OK;

		 }
	

		 iret = ks_shell_str2uint(argv[3], &data_width);
		 if (iret != 0)
		 {
			 return CMD_ERR_PARAMS_FORMAT;
		 }

		 if(argc  == 4 ){
			 ks_driver_uart_config_ex(uart_id, baud,data_width,ks_driver_uart_get_parity(uart_id),ks_driver_uart_get_stopbit(uart_id));
			 ks_shell_printf(ctx->uart_id, "uart%d	config baud %d data_width %d parity %d stop_bits %d !!!\r\n",uart_id, ks_driver_uart_get_baud(uart_id),ks_driver_uart_get_datawidth(uart_id),ks_driver_uart_get_parity(uart_id),ks_driver_uart_get_stopbit(uart_id));
			 return CMD_ERR_OK;
		 }
		 

		 iret = ks_shell_str2uint(argv[4], &parity);
		 if (iret != 0)
		 {
			 return CMD_ERR_PARAMS_FORMAT;
		 }

		 iret = ks_shell_str2uint(argv[5], &stop_bits);
		 if (iret != 0)
		 {
			 return CMD_ERR_PARAMS_FORMAT;
		 }
		 
		 ks_driver_uart_config_ex(uart_id, baud,data_width,parity,stop_bits);
		 
		 ks_shell_printf(ctx->uart_id, "uart%d  config baud %d  data_width %d parity %d  stop_bits %d !!!\r\n",uart_id, baud,data_width,parity,stop_bits);
    }
    else
    {
        return CMD_ERR_PARAMS_NOT_ENOUGH;
    }

    return 0;
}

static void timer_uart_cb(void* arg){


	ks_os_flag_set_bits(flag_handle_uart_task, 1, SET_FLAG_MODE_OR);

}

static int uart_tick_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
	uint32_t enable;
	static int uart_id = 0;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &enable);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}
	else
	{
	  	enable = 1;
	}
	if(enable){
		if(timer_uart == 0){
			ks_os_timer_coarse_create(&timer_uart, "timer_uart", timer_uart_cb, (void*)&uart_id, 100, 1000, 0);
		}
		uart_id = ctx->uart_id;

		ks_os_timer_coarse_activate(timer_uart);
	}else{
		if(timer_uart != 0)
		ks_os_timer_coarse_deactivate(timer_uart);
	}

	ks_shell_printf(ctx->uart_id,"uart_tick_cmd : %d \r\n",enable);

    return 0;
}


void TestTxSpeedTask(void *p_arg)
{

	uint32_t len = strlen(s_tx_str);
	char* p_data = s_tx_str;
 	int uart_id = (int)p_arg;
	
	uint32_t xfer_bytes = 0;
	uint64_t check_ticks ,start_ticks,end_ticks ;
	check_ticks = 0;
	uint64_t xfer_speed;
	uint64_t time_clock = ks_os_get_sys_clock();
	while(1)
    {	

		if(s_tx_enbale == 0){
			ks_os_thread_sleep(10);
			continue;
		}
		start_ticks = ks_os_get_free_time();
    	int size = ks_driver_uart_get_tx_ringbuffer_offset(s_tx_uartid);
		if(size < 30*1024){
			
			ks_driver_uart_send_buffer(s_tx_uartid, (U8*)p_data, (int)len);
			xfer_bytes += len;
		}
		end_ticks = ks_os_get_free_time();
		check_ticks += (end_ticks - start_ticks);

		if(check_ticks > time_clock){
		
			xfer_speed = xfer_bytes / (check_ticks / time_clock);
			//float write_speed = 1.0f * xfer_bytes / (1.0f * check_ticks / ks_os_get_sys_clock());
	   		ks_shell_printf(uart_id, "xfer_speed: %llu B/s \r\n", xfer_speed );
	   		//ks_shell_printf(uart_id, "xfer_speed:  %.2f KB/s \r\n", write_speed/1000/1000 );
			check_ticks = 0;
			xfer_bytes = 0;

		}
	}
}

static int uart_txspeed_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
	uint32_t enable;
	
	s_tx_uartid = 1 ;
	s_tx_enbale = 0;

	if (argc >= 2 )
	{
		iret = ks_shell_str2uint(argv[1], &s_tx_uartid);
		if (iret != 0)
		{
			return CMD_ERR_PARAMS_FORMAT;
		}

		if(argc > 2 ){
			iret = ks_shell_str2uint(argv[2], &s_tx_enbale);
			if (iret != 0)
			{
				return CMD_ERR_PARAMS_FORMAT;
			}
		}
		
	}
	else
	{
		ks_shell_printf(ctx->uart_id,"dmatx : s_tx_total_count %d  strlen(s_tx_str) %d \r\n",s_tx_total_count,strlen(s_tx_str));
	}


	ks_shell_printf(ctx->uart_id,"uart_dmatx_cmd : s_tx_uartid %d  s_tx_enbale %d \r\n",s_tx_uartid,s_tx_enbale);

	if(thread_handle_dmatx_task == 0 ){
		ks_os_thread_create(&thread_handle_dmatx_task,
								 "TestTxSpeedTask",					
								 TestTxSpeedTask,						
								 (void*)ctx->uart_id,
								 30,								
								 thread_stack_dmatx_task,			
								 sizeof(thread_stack_dmatx_task),	
								 0,
								 1
								 );
	}


    return 0;
}



int  uart_rx_input_handler(int uart_id, char byte)
{

	static int index;
	U8* buffer = g_rx_buffer;
	
	buffer[index] = (char)byte;	

	if(byte == '\n'){
		ks_os_printf(0,"recv %d \r\n",index);
		ks_shell_printf_dump_hex(0,  buffer, index);
		index = 0;
	}else{
		index ++;
	}

	return 0;
}


int  uart_txrx_loop_input_handler(int uart_id, char byte)
{

	static int index;
	U8* buffer = g_rx_buffer;
	
	buffer[index] = (char)byte;	

	if(byte == '\n'){
		
		//ks_os_printf(0,"recv %d \r\n",index);

		index = 0;

		for (int i = 0; i < txrx_len; i++) {
			if (buffer[i] !=  (i%10+'0') ) {
				ks_shell_printf(0,"data error  i %d dst %d src %d \r\n",i,buffer[i],i&0xFF);
				ks_shell_printf_dump_hex(0,  buffer,txrx_len);
				return 0;
	        }
		}
		
		ks_os_flag_set_bits(flag_handle_txrx_loop_task, 1, SET_FLAG_MODE_OR);


	}else{
		index ++;
	}
	
	return 0;
}


void uart_txrx_timer_loop_cb(void *p_arg){
	int curent_count = *(int*) p_arg;
	static int last_count = 0;
	if(curent_count!=0 ){
		if(curent_count > last_count ){
			ks_shell_printf(0,"txrx send: %d \r\n",curent_count);
			ks_shell_printf(0,"\r\n");
			last_count = curent_count;
		}
	}

}
void uart_tx_timer_loop_cb(void *p_arg){
	int curent_count = *(int*) p_arg;
	static int last_count = 0;
	if(curent_count!=0 ){
		if(curent_count > last_count ){
			ks_shell_printf(0,"send: %d \r\n",curent_count);
			ks_shell_printf(0,"\r\n");
			last_count = curent_count;
		}
	}

}

void uart_rx_timer_loop_cb(void *p_arg){
	int curent_count = *(int*) p_arg;
	static int last_count = 0;
	
	if(curent_count!=0 ){
		if(curent_count > last_count ){
			ks_shell_printf(0,"recv: %d \r\n",curent_count);
			ks_shell_printf(0,"\r\n");
			last_count = curent_count;
		}
	}

}


void UartTxRxLoopTask(void *p_arg)
{

	int  len = (int)p_arg;
	uint32_t speed;
	int count = 0;
	U8* buffer = g_tx_buffer;
	
	if(len > TXRX_LOOP_LEN){
		len = TXRX_LOOP_LEN;
	}
	
	for (int i = 0; i < len ; i++) {
    	buffer[i] = i%10+'0';
	}
	
	buffer[len] = '\n';

	txrx_len = len;

	int iret =0;
	
	ks_shell_uart_enable(1,0);
	ks_shell_uart_enable(2,0);
	
	ks_driver_uart_register_protocol(1,uart_txrx_loop_input_handler);
	//ks_driver_uart_register_protocol(2,uart_rx_input_handler);

	ks_shell_printf(0," tx rx task  has start len %d  !!! \r\n",len);
	
	U32 ret = ks_os_timer_coarse_create(&timer_loop, "timer_loop", uart_txrx_timer_loop_cb, &count, 100, 1000, 1);
	
	ks_os_flag_set_bits(flag_handle_txrx_loop_task, 1, SET_FLAG_MODE_OR);
	
	while(1)
    {
    	U32 status = ks_os_flag_wait(flag_handle_txrx_loop_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
	
		ks_driver_uart_send_buffer(2,buffer,len+1);
		
		if(iret!=0){
			ks_shell_printf(0,"ks_driver_uart_send_buffer error: %d \r\n",iret);
			//ks_os_timer_coarse_deactivate(timer_loop);
			break;
		}else{

			count++;
		}
		

    }
}

static int uart_txrx_loop_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t speed,txlen;
	int iret ;
	if (argc >= 2 )
	{

		
		iret = ks_shell_str2uint(argv[1], &s_tx_uartid);
		if (iret != 0)
		{
			return CMD_ERR_PARAMS_FORMAT;
		}
	   iret = ks_shell_str2uint(argv[1], &txlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(txlen > sizeof(g_tx_buffer)) txlen = TXRX_LOOP_LEN ;

	}else{
		txlen = TXRX_LOOP_LEN ;
	}


	if(thread_handle_txrx_loop_task!=0){
		
		ks_shell_printf(ctx->uart_id,"txrx_loop_task has create !!! \r\n");

		return -1;
	}
	
	ks_os_flag_create(&flag_handle_txrx_loop_task, "uart_tx_rx_flag");
	ks_os_thread_create(&thread_handle_txrx_loop_task,			
						 "UartTxRxLoopTask",
						 UartTxRxLoopTask,						
						 (void*)(txlen),
						 15,								
						 thread_stack_txrx_loop_task,			
						 sizeof(thread_stack_txrx_loop_task),	
						 0,
						 1
						 );


	return 0;

}


static int uart_rx_dump_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;

	uint32_t enable;
	uint32_t uart_id = 0;


	if (argc >= 2 )
	{
		iret = ks_shell_str2uint(argv[1], &uart_id);
		if (iret != 0)
		{
			return CMD_ERR_PARAMS_FORMAT;
		}

		if(argc > 2 ){
			iret = ks_shell_str2uint(argv[2], &enable);
			if (iret != 0)
			{
				return CMD_ERR_PARAMS_FORMAT;
			}
		}
		
	}
	else
	{
		enable = 1;
	}

	ks_shell_printf(ctx->uart_id,"uart_dump : uart_id %d  enable %d \r\n",uart_id,enable);

	if(enable){
		ks_driver_uart_register_protocol(uart_id,uart_rx_input_handler);
	}else{
		ks_driver_uart_deregister_protocol(uart_id,uart_rx_input_handler);
	}	

	return 0;

}


void UartTxLoopTask(void *p_arg)
{

	int count = 0;
	int iret =0;
	
	ks_shell_uart_enable(s_tx_uartid,0);

	ks_shell_printf(0,"tx loop task  has start  tx_uartid %d  baud  %d !!! \r\n",s_tx_uartid,ks_driver_uart_get_baud(s_tx_uartid));
	
	U32 ret = ks_os_timer_coarse_create(&timer_loop, "timer_loop", uart_tx_timer_loop_cb, &count, 100, 1000, 1);

	while(1)
    {
    	//U32 status = ks_os_flag_wait(flag_handle_rx_loop_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
		ks_os_thread_sleep(10);
		
		if(s_tx_loop_enable == 0 ) continue;

		ks_driver_uart_send_string(s_tx_uartid,s_tx_str);
		
		count++;

    }
}


static int uart_tx_loop_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t speed,txlen;
	int iret ;
	if (argc >= 2 )
	{
		iret = ks_shell_str2uint(argv[1], &s_tx_uartid);
		if (iret != 0)
		{
			return CMD_ERR_PARAMS_FORMAT;
		}

		if(argc > 2 ){
			iret = ks_shell_str2uint(argv[2], &s_tx_loop_enable);
			if (iret != 0)
			{
				return CMD_ERR_PARAMS_FORMAT;
			}
		}

	}else{
		return -1 ;
	}


	if(thread_handle_tx_loop_task!=0){
		
		ks_shell_printf(ctx->uart_id,"tx_loop_task has create !!! \r\n");

		return -1;
	}
	
	ks_os_flag_create(&flag_handle_tx_loop_task, "uart_tx_flag");
	ks_os_thread_create(&thread_handle_tx_loop_task,			
						 "UartTxLoopTask",
						 UartTxLoopTask,						
						 0,
						 15,								
						 thread_stack_tx_loop_task,			
						 sizeof(thread_stack_tx_loop_task),	
						 0,
						 1
						 );


	return 0;

}

int  uart_rx_loop_input_handler(int uart_id, char byte)
{

	static U32 index;
	U8* buffer = g_rx_buffer;
	
	if(s_rx_loop_enable == 0 ) return 0 ;
	
	if(index < sizeof(g_rx_buffer))
	buffer[index] = (char)byte;	
	

	if(byte == '^'){
		for (int i = 0; i < (int)strlen(s_tx_str); i++) {
			if (buffer[i] != s_tx_str[i] ) {
				ks_os_printf(0,"recv %d \r\n",index);
				ks_shell_printf(0,"data error  i %d dst %d src %d \r\n",i,buffer[i],s_tx_str[i]);
				ks_shell_printf_dump_hex(0,  buffer,index);
				ks_driver_uart_deregister_protocol(s_rx_uartid,uart_rx_loop_input_handler);
				return 0;
	        }
		}
		index = 0;
		ks_os_flag_set_bits(flag_handle_rx_loop_task, 1, SET_FLAG_MODE_OR);
	}else{
		index ++;
	}
	
	return 0;
}

void UartRxLoopTask(void *p_arg)
{
	int count = 0;

	int iret =0;
	
		
	ks_shell_uart_enable(s_rx_uartid,0);
	

	ks_driver_uart_register_protocol(s_rx_uartid,uart_rx_loop_input_handler);


	ks_shell_printf(0,"rx task  has start  rx_uartid %d  baud  %d !!! \r\n",s_rx_uartid,ks_driver_uart_get_baud(s_rx_uartid));
	
	U32 ret = ks_os_timer_coarse_create(&timer_loop, "timer_loop", uart_rx_timer_loop_cb, &count, 100, 1000, 1);

	while(1)
    {
    	U32 status = ks_os_flag_wait(flag_handle_rx_loop_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
	
		count++;
    }
}


static int uart_rx_loop_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t speed,txlen;
	int iret ;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &s_rx_uartid);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   
	   if (argc >2 ){
	   	
		   iret = ks_shell_str2uint(argv[2], &s_rx_loop_enable);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
	   }

	}else{
		return -1;
	}


	if(thread_handle_rx_loop_task!=0){
		
		ks_shell_printf(ctx->uart_id,"rx_loop_task has create !!! \r\n");

		return -1;
	}
	
	ks_os_flag_create(&flag_handle_rx_loop_task, "uart_rx_flag");
	ks_os_thread_create(&thread_handle_rx_loop_task,			
						 "UartRxLoopTask",
						 UartRxLoopTask,						
						 0,
						 15,								
						 thread_stack_rx_loop_task,			
						 sizeof(thread_stack_rx_loop_task),	
						 0,
						 1
						 );


	return 0;

}


static cmd_proc_t uart_demo_cmds[] = {
	{.cmd = "uart_echo", .fn = uart_echo_cmd,  .help = "uart_echo <uartid> <string>"},
    {.cmd = "uart_baud", .fn = uart_baud_cmd,  .help = "uart_baud <uartid> <baud>"},
	{.cmd = "uart_config", .fn = uart_config_cmd,  .help = "uart_config <uartid> <baud> <data_width> <parity> <stop_bits>"},
	{.cmd = "tick", .fn = uart_tick_cmd,  .help = "tick <enable/disable> "},
    {.cmd = "rx_dump", .fn = uart_rx_dump_cmd,  .help = "rx_dump <uartid> <enable>"},
	{.cmd = "tx_loop", .fn = uart_tx_loop_cmd,  .help = "tx_loop  <uartid>  <len>"},
	{.cmd = "rx_loop", .fn = uart_rx_loop_cmd,  .help = "rx_loop  <uartid>  <len>"},
	//{.cmd = "tx_speed", .fn = uart_txspeed_cmd,  .help = "tx_speed <uartid> <enable/disable> "},
	//{.cmd = "txrx_loop", .fn = uart_txrx_loop_cmd,  .help = "txrx_loop  <len>"},
};


void UartDemoStartInfoPrint()
{
	char t_string[128];
	sprintf(t_string, "UartDemo Start, COM0\r\n");
	ks_driver_uart_send_string(0, t_string);		
	sprintf(t_string, "UartDemo Start, COM1\r\n");
	ks_driver_uart_send_string(1, t_string);		
	sprintf(t_string, "UartDemo Start, COM2\r\n");
	ks_driver_uart_send_string(2, t_string);	
	sprintf(t_string, "UartDemo Start, COM3\r\n");
	ks_driver_uart_send_string(3, t_string);	

}

void UartDemoTask(void *p_arg)
{
	char t_string[128];
	ks_os_thread_vfp_enable();

	ks_shell_add_cmds(uart_demo_cmds, sizeof(uart_demo_cmds) / sizeof(cmd_proc_t));

	UartDemoStartInfoPrint();
	

	while(1)
    {
    	U32 status = ks_os_flag_wait(flag_handle_uart_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);

		sprintf(t_string, "this uart0 tick echo \r\n");
		ks_driver_uart_send_string(0, t_string);		
		sprintf(t_string, "this uart1 tick echo \r\n");
		ks_driver_uart_send_string(1, t_string);		
		sprintf(t_string, "this uart2 tick echo \r\n");
		ks_driver_uart_send_string(2, t_string);	
		sprintf(t_string, "this uart3 tick echo \r\n");
		ks_driver_uart_send_string(3, t_string);	
    }
}


void UartDemoInit(void)
{

	ks_os_flag_create(&flag_handle_uart_task, "uart_demo_task");
	ks_os_thread_create(&thread_handle_uart_task,			
							 "uart_demo_task",					
							 UartDemoTask,						
							 0,								
							 15,								
							 thread_stack_uart_task,			
							 sizeof(thread_stack_uart_task),	
							 0,
							 1
							 );

	
}




