
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  spi.c
 *
 * @brief  spi 演示实现
 */
#include <stdint.h>
#include <stdlib.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "ks_spi.h"
#include "spi_demo.h"
#include "ks_shell.h"

#define  STACK_SIZE_SPI_TASK          1024
static S32 thread_stack_spi_task[STACK_SIZE_SPI_TASK];
static OSHandle flag_handle_spi_task;
static OSHandle thread_handle_spi_task;

static S32 thread_stack_spi_master_task[STACK_SIZE_SPI_TASK];
static OSHandle flag_handle_spi_master_task;
static OSHandle thread_handle_spi_master_task;




static S32 thread_stack_spi_slave_task[STACK_SIZE_SPI_TASK];
static OSHandle flag_handle_spi_slave_task;
static OSHandle thread_handle_spi_slave_task;


static OSHandle timer_master_task;


static U8 g_tx_buffer[1024] __attribute__ ((aligned (64)));
static U8 g_rx_buffer[1024] __attribute__ ((aligned (64)));

#define  TEST_BUFFER_LEN          64

static void timer_master_cb(void* arg){
	int uart_id = 0;
	uart_id = (*(int*)arg)<<8;

	ks_os_flag_set_bits(flag_handle_spi_master_task, uart_id|1, SET_FLAG_MODE_OR);

}


int spi_master_send_loop(U8* txbuffer,int len)
{

	int iret = ks_driver_spi_master_send(0,txbuffer,len,1000);
	if(iret ==0){
		ks_shell_printf(0,"master send:\r\n");
		ks_shell_printf_dump_hex(0,txbuffer,len);
		ks_shell_printf(0," \r\n");
	
	}else{
		ks_shell_printf(0," ks_driver_spi_master_send  %d  !!! \r\n",iret);
		return -1;
	
	}
	
	//让出CPU   让 log 打印
	ks_os_thread_sleep(2);
	return 0;
}

int spi_master_recieve_loop(U8* rxbuffer,int len)
{
	memset(rxbuffer,0,len);
	int iret = ks_driver_spi_master_receive(0,rxbuffer,len,KS_OS_WAIT_FOREVER);
	if(iret ==0){
	
		for (int i = 0; i < len; i++) {
			if (rxbuffer[i] != (U8)(i&0xFF) ) {
				ks_shell_printf(0,"data error  i %d dst %d src %d \r\n",i,rxbuffer[i],i&0xFF);
				return -1;
			}
		}
		ks_shell_printf(0,"master receive:\r\n");
		ks_shell_printf_dump_hex(0,rxbuffer,len);
		ks_shell_printf(0," \r\n");
	}else{
		ks_shell_printf(0," ks_driver_spi_master_receive  %d  !!! \r\n",iret);
		return -1;
	
	}
	//让出CPU   让 log 打印
	ks_os_thread_sleep(2);
	return 0;
}

int spi_master_transfer_loop(U8* txbuffer,U8* rxbuffer,int len)
{
	memset(rxbuffer,0,len);
	int iret = ks_driver_spi_master_transfer(0,txbuffer,rxbuffer,len,len,1000);
	if(iret ==0){
	
		for (int i = 0; i < len; i++) {
			if (rxbuffer[i] != (U8)(i&0xFF) ) {
				ks_shell_printf(0,"data error  i %d dst %d src %d \r\n",i,rxbuffer[i],i&0xFF);
				return -1;
			}
		}
		ks_shell_printf(0,"master receive:\r\n");
		ks_shell_printf_dump_hex(0,rxbuffer,len);
		ks_shell_printf(0," \r\n");
	
	}else{
		ks_shell_printf(0," ks_driver_spi_master_transfer  %d  !!! \r\n",iret);
		return -1;
	
	}
	//让出CPU   让 log 打印
	ks_os_thread_sleep(2);
	return 0;

}

void SpiMasterTask(void *p_arg)
{

	U8 reg=0;
	int iret ;
	int len= 1024;
	U8* rxbuffer = g_rx_buffer;
	U8* txbuffer = g_tx_buffer;;


	for (uint32_t i = 0; i < 1024; i++) {
    	txbuffer[i] = i&0xFF;
	}

	ks_os_thread_vfp_enable();
	
	ks_driver_spi_master_init(0);
	
	spi_config_t config;
	config.baud = 1*1000000;
	config.mode = SPI_MODE_MASTER;
	config.format = SPI_FORMAT_CPOL0_CPHA0;
	config.bit_width = 8;

	ks_driver_spi_master_config(0,&config);
	
	ks_shell_printf(0," master task  has start baud %dMbps !!! \r\n",config.baud/1000000);
	
	while(1)
    {

		iret = spi_master_send_loop(txbuffer,len);
		//iret = spi_master_recieve_loop(rxbuffer,len);
		//iret = spi_master_transfer_loop(txbuffer,rxbuffer,len);
		if(iret != 0){
			return;
		}

    }
}


int spi_slave_send_loop(U8* txbuffer,int len)
{

	int iret = ks_driver_spi_slave_send(0,txbuffer,len,KS_OS_WAIT_FOREVER);
	if(iret ==0){
		ks_shell_printf(0,"slave send:\r\n");
		ks_shell_printf_dump_hex(0,txbuffer,len);
		ks_shell_printf(0," \r\n");
	
	}else{
		ks_shell_printf(0," ks_driver_spi_slave_send  %d  !!! \r\n",iret);
		return -1;
	
	}
	
	//让出CPU   让 log 打印
	ks_os_thread_sleep(1);
	return 0;
}

int spi_slave_recieve_loop(U8* rxbuffer,int len)
{
	memset(rxbuffer,0,len);
	int iret = ks_driver_spi_slave_receive(0,rxbuffer,len,KS_OS_WAIT_FOREVER);
	if(iret ==0){
	
		for (int i = 0; i < len; i++) {
			if (rxbuffer[i] != (U8)(i&0xFF) ) {
				ks_shell_printf(0,"data error  i %d dst %d src %d \r\n",i,rxbuffer[i],(U8)i&0xFF);
				return -1;
			}
		}
		ks_shell_printf(0,"slave receive:\r\n");
		ks_shell_printf_dump_hex(0,rxbuffer,len);
		ks_shell_printf(0," \r\n");
	
	}else{
		ks_shell_printf(0," ks_driver_spi_slave_receive  %d  !!! \r\n",iret);
		return -1;
	
	}
	ks_os_thread_sleep(1);
	return 0;
}

int spi_slave_transfer_loop(U8* txbuffer,U8* rxbuffer,int len)
{
	memset(rxbuffer,0,len);
	int iret = ks_driver_spi_slave_transfer(0,txbuffer,rxbuffer,len,len,KS_OS_WAIT_FOREVER);
	if(iret ==0){
	
		for (int i = 0; i < len; i++) {
			if (rxbuffer[i] != (U8)(i&0xFF) ) {
				ks_shell_printf(0,"data error  i %d dst %d src %d \r\n",i,rxbuffer[i],i&0xFF);
				return -1;
			}
		}
		ks_shell_printf(0,"slave receive:\r\n");
		ks_shell_printf_dump_hex(0,rxbuffer,len);
		ks_shell_printf(0," \r\n");
	
	}else{
		ks_shell_printf(0," ks_driver_spi_slave_transfer  %d  !!! \r\n",iret);
		return -1;
	
	}
	//让出CPU   让 log 打印
	ks_os_thread_sleep(1);
	return 0;
}


void SpiSlaveTask(void *p_arg)
{

	U8 reg=0;
	int len= 1024;


	U8* rxbuffer = g_rx_buffer;
	U8* txbuffer = g_tx_buffer;

	for (uint32_t i = 0; i < 1024; i++) {
    	txbuffer[i] = i&0xFF;
	}

	int iret ;
	ks_driver_spi_slave_init(0);
	
	spi_config_t config;
	config.baud = 1*1000000;
	config.mode = SPI_MODE_SLAVE;
	config.format = SPI_FORMAT_CPOL0_CPHA0;
	config.bit_width = 8;
	
	ks_driver_spi_slave_config(0,&config);

	ks_shell_printf(0,"slave  task  has start baud %dMbps !!! \r\n",config.baud/1000000 );
	
	while(1)
    {

		iret = spi_slave_recieve_loop(rxbuffer,len);
		//iret = spi_slave_send_loop(txbuffer,len);
		//iret = spi_slave_transfer_loop(txbuffer,rxbuffer,len);
		if(iret != 0){
			return;
		}

    }
}

static int spi_master_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	if(thread_handle_spi_master_task!=0){
		
		ks_shell_printf(ctx->uart_id,"spi_master_task has create !!! \r\n");

		return -1;
	}
	
	ks_os_flag_create(&flag_handle_spi_master_task, "spi_master_flag");
	ks_os_thread_create(&thread_handle_spi_master_task,			
						 "spi_master_task",					
						 SpiMasterTask,						
						 0,
						 15,								
						 thread_stack_spi_master_task,			
						 sizeof(thread_stack_spi_master_task),	
						 0,
						 1
						 );


    return 0;
}


static int spi_slave_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	if(thread_handle_spi_slave_task!=0){
		
		ks_shell_printf(ctx->uart_id,"spi_master_slave_task has create !!! \r\n");

		return -1;
	}

	ks_os_thread_create(&thread_handle_spi_slave_task,			
						 "spi_slave_task",					
						 SpiSlaveTask,						
						 0,
						 15,								
						 thread_stack_spi_slave_task,			
						 sizeof(thread_stack_spi_slave_task),	
						 0,
						 1
						 );



    return 0;
}




static int master_send_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int iret;
	uint32_t  txlen;
	U8 reg=0;
	U8* buffer= g_tx_buffer;
	
	for (uint32_t i = 0; i < 1024; i++) {
    	buffer[i] = i&0xFF;
	}

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &txlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(txlen > 1024 ) txlen = TEST_BUFFER_LEN ;
	   
	}else{
		txlen = TEST_BUFFER_LEN ;
	}

	ks_driver_spi_master_init(0);

	spi_config_t config;
	config.baud = 100000;
	config.mode = SPI_MODE_MASTER;
	config.format = SPI_FORMAT_CPOL0_CPHA0;
	config.bit_width = 8;

	
	ks_driver_spi_master_config(0,&config);

	ks_shell_printf(ctx->uart_id,"master_send_cmd txlen %d \r\n",txlen);
	
	int ret =ks_driver_spi_master_send(0,buffer,txlen,10000);
	if(ret!=0){
		ks_shell_printf(0,"ks_driver_spi_master_send : %d \r\n",ret);
	}else{
		ks_shell_printf(0,"master send:\r\n");
		ks_shell_printf_dump_hex(0,buffer,txlen);
	}


	return 0;

}



static int slave_send_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int iret;
	uint32_t  txlen;
	U8* buffer= g_tx_buffer;
	
	for (uint32_t i = 0; i < 1024; i++) {
    	buffer[i] = i&0xFF;
	}

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &txlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(txlen > 1024 ) txlen = TEST_BUFFER_LEN ;
	   
	}else{
		txlen = TEST_BUFFER_LEN ;
	}
	ks_shell_printf(ctx->uart_id,"slave_send_cmd txlen %d \r\n",txlen);
	

	ks_driver_spi_slave_init(0);

	spi_config_t config;
	config.baud = 100000;
	config.mode = SPI_MODE_SLAVE;
	config.format = SPI_FORMAT_CPOL0_CPHA0;
	config.bit_width = 8;

	
	ks_driver_spi_slave_config(0,&config);

	int ret = ks_driver_spi_slave_send(0,buffer,txlen,10000);
	if(ret!=0){
		ks_shell_printf(0,"ks_driver_spi_slave_send : %d \r\n",ret);
	}else{
		ks_shell_printf(0,"slave send:\r\n");
		ks_shell_printf_dump_hex(0,buffer,txlen);
	}

	return 0;
}

static int master_receive_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	
	int iret;
	uint32_t  rxlen;

	U8* buffer= g_rx_buffer;


	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &rxlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(rxlen > 1024 ) rxlen = TEST_BUFFER_LEN ;
	   
	}else{
		rxlen = TEST_BUFFER_LEN ;
	}

	
	ks_driver_spi_master_init(0);

	spi_config_t config;
	config.baud = 100000;
	config.mode = SPI_MODE_MASTER;
	config.format = SPI_FORMAT_CPOL0_CPHA0;
	config.bit_width = 8;
	
	ks_driver_spi_master_config(0,&config);

	ks_shell_printf(ctx->uart_id,"master start receive rxlen %d \r\n",rxlen);

	int ret = ks_driver_spi_master_receive(0,buffer,rxlen,15000);
	

	ks_shell_printf(ctx->uart_id,"master_receive_cmd  receive ret %d \r\n",ret);

	if(ret == 0){
		ks_shell_printf_dump_hex(ctx->uart_id,buffer,rxlen);
	}


	return 0;

}

static int slave_receive_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int iret;
	uint32_t  rxlen;
	U8* buffer= g_rx_buffer;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &rxlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(rxlen > 1024 ) rxlen = TEST_BUFFER_LEN ;
	   
	}else{
		rxlen = TEST_BUFFER_LEN ;
	}

	ks_driver_spi_slave_init(0);

	spi_config_t config;
	config.baud = 100000;
	config.mode = SPI_MODE_SLAVE;
	config.format = SPI_FORMAT_CPOL0_CPHA0;
	config.bit_width = 8;

	
	ks_driver_spi_slave_config(0,&config);

	ks_shell_printf(ctx->uart_id,"slave start receive rxlen %d \r\n",rxlen);
	//slave rx during 0-15 s
	int ret = ks_driver_spi_slave_receive(0,buffer,rxlen,15000);

	ks_shell_printf(ctx->uart_id,"slave_receive_cmd  receive ret %d \r\n",ret);

	if(ret == 0){
		ks_shell_printf_dump_hex(ctx->uart_id,buffer,rxlen);
	}


	return 0;
}


static int master_transfer_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	
	int iret;


	uint32_t  tslen;
	U8* rxbuffer = g_rx_buffer;
	U8* txbuffer = g_tx_buffer;


	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &tslen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(tslen > 512 ) tslen = TEST_BUFFER_LEN ;
	   
	}else{
		tslen = TEST_BUFFER_LEN ;
	}

	
	for (uint32_t i = 0; i < tslen; i++) {
    	txbuffer[i] = i&0xFF;
	}
	memset(rxbuffer,0,tslen);
	ks_driver_spi_master_init(0);

	spi_config_t config;
	config.baud = 100000;
	config.mode = SPI_MODE_MASTER;
	config.format = SPI_FORMAT_CPOL0_CPHA0;
	config.bit_width = 8;
	
	ks_driver_spi_master_config(0,&config);

	ks_shell_printf(ctx->uart_id,"master start transfer tslen %d \r\n",tslen);

	int ret = ks_driver_spi_master_transfer(0,txbuffer,rxbuffer,tslen,tslen,15000);
	

	ks_shell_printf(ctx->uart_id,"master_transfer_cmd   ret %d \r\n",ret);

	if(ret == 0){
		ks_shell_printf_dump_hex(ctx->uart_id,rxbuffer,tslen);
	}


	return 0;

}

static int slave_transfer_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int iret;
	uint32_t  tslen;
	U8* rxbuffer = g_rx_buffer;
	U8* txbuffer = g_tx_buffer;


	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &tslen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(tslen > 512 ) tslen = TEST_BUFFER_LEN ;
	   
	}else{
		tslen = TEST_BUFFER_LEN ;
	}

	
	for (uint32_t i = 0; i < tslen; i++) {
    	txbuffer[i] = i&0xFF;
	}
	memset(rxbuffer,0,tslen);

	ks_driver_spi_slave_init(0);

	spi_config_t config;
	config.baud = 100000;
	config.mode = SPI_MODE_SLAVE;
	config.format = SPI_FORMAT_CPOL0_CPHA0;
	config.bit_width = 8;

	
	ks_driver_spi_slave_config(0,&config);

	ks_shell_printf(ctx->uart_id,"slave start transfer tslen %d \r\n",tslen);
	//slave rx during 0-15 s
	int ret = ks_driver_spi_slave_transfer(0,txbuffer,rxbuffer,tslen,tslen,15000);

	ks_shell_printf(ctx->uart_id,"slave_transfer_cmd   ret %d \r\n",ret);

	if(ret == 0){
		ks_shell_printf_dump_hex(ctx->uart_id,rxbuffer,tslen);
	}


	return 0;
}

static cmd_proc_t spi_cmds[] = {

	{.cmd = "m_tx", .fn = master_send_cmd, .help = "m_tx  <txlen>"},
	{.cmd = "m_rx", .fn = master_receive_cmd, .help = "m_rx <rxlen> "},
	{.cmd = "m_ts", .fn = master_transfer_cmd, .help = "m_ts <tslen> "},
    {.cmd = "s_tx", .fn = slave_send_cmd, .help = "s_tx <txlen>"},
    {.cmd = "s_rx", .fn = slave_receive_cmd, .help = "s_rx <rxlen>"},
    {.cmd = "s_ts", .fn = slave_transfer_cmd, .help = "s_ts <tslen> "},
	{.cmd = "master",.fn = spi_master_cmd,   .help="master "},
	{.cmd = "slave",.fn = spi_slave_cmd,  .help="slave"},


};

void SpiDemoTask(void *p_arg)
{
	ks_os_thread_vfp_enable();


	
	ks_shell_add_cmds(spi_cmds, sizeof(spi_cmds)/ sizeof(cmd_proc_t) );

	while(1)
    {
    	U32 status = ks_os_flag_wait(flag_handle_spi_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);


    }
}

void SpiDemoInit(void)
{

	ks_os_flag_create(&flag_handle_spi_task, "spi_demo_task");
	ks_os_thread_create(&thread_handle_spi_task,			
							 "spi_demo_task",					
							 SpiDemoTask,						
							 0,								
							 15,								
							 thread_stack_spi_task,			
							 sizeof(thread_stack_spi_task),	
							 0,
							 1
							 );

	
}




