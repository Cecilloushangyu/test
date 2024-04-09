
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  i2c_demo.c
 *
 * @brief  i2c 演示实现
 */


#include <stdlib.h>
#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "ks_shell.h"
#include "ks_i2c.h"
#include "i2c_demo.h"


#define  STACK_SIZE_I2C_TASK          1024
static S32 thread_stack_i2c_task[STACK_SIZE_I2C_TASK];
static OSHandle flag_handle_i2c_task;
static OSHandle thread_handle_i2c_task;



static S32 thread_stack_i2c_master_task[STACK_SIZE_I2C_TASK];
static OSHandle flag_handle_i2c_master_task;
static OSHandle thread_handle_i2c_master_task;


static S32 thread_stack_i2c_slave_task[STACK_SIZE_I2C_TASK];
static OSHandle flag_handle_i2c_slave_task;
static OSHandle thread_handle_i2c_slave_task;


static S32 thread_stack_i2c_slave_check_task[STACK_SIZE_I2C_TASK];
static OSHandle flag_handle_i2c_slave_check_task;
static OSHandle thread_handle_i2c_slave_check_task;
static OSHandle flag_handle_i2c_check_task;

typedef struct
{
	U8  check_req;
	U8  check_ret;
	U8* current_rx_buffer;
}i2c_rx_check_param;

static i2c_rx_check_param   s_i2c_rx_check;



#define TX_BUFFER_SIZE 10240
#define RX_BUFFER_SIZE 10240


static U8 g_tx_buffer[TX_BUFFER_SIZE+64] __attribute__ ((aligned (64)));
static U8 g_rx_buffer[RX_BUFFER_SIZE+64] __attribute__ ((aligned (64)));


static OSHandle timer_rtc ;
static OSHandle timer_loop;

//#define BUFFER_START_INDEX 0
#define BUFFER_START_INDEX 1


#define RX8803_ADDR 0x32

#define MASTER_ADDR 0x55
#define SLAVE_ADDR 	0x66

//寄存器地址
#define	RX8803_ADDR_SECONDS			0x0			//秒	
#define	RX8803_ADDR_MINUTE			0x1			//分
#define	RX8803_ADDR_HOUR			0x2			//时
#define	RX8803_ADDR_WEEK			0x3			//星期	
#define	RX8803_ADDR_DAY				0x4			//日
#define	RX8803_ADDR_MONTH			0x5			//月
#define	RX8803_ADDR_YEAR			0x6			//年

#define	RX8803_ADDR_FLAG			0xE
#define	RX8803_ADDR_CONTROL			0xF

#define  TEST_BUFFER_LEN          64

#define  TEST_DMA_BUFFER_LEN      256


typedef struct
{
	U8 day;
	U8 month;
	U8 year; // 0-99 范围

}RX8803_Date;


typedef struct
{
	U8 second;
	U8 minute;
	U8 hour;
}RX8803_Time;


U8 RtcSetDate(RX8803_Date* t)
{
    U8 reg=0;
    U8 rtc_str;
	U8 buffer[2];

	reg = RX8803_ADDR_DAY;
	rtc_str= ((t->day/10)<<4) | (t->day%10);

	buffer[0] = reg;
	buffer[1] = rtc_str;
	ks_driver_i2c_master_send(0,RX8803_ADDR,buffer,2,50);

	ks_os_poll_delay_msec(1);
	
	reg = RX8803_ADDR_MONTH;
	rtc_str = ((t->month/10)<<4) | (t->month%10);
	buffer[0] = reg;
	buffer[1] = rtc_str;
	ks_driver_i2c_master_send(0,RX8803_ADDR,buffer,2,50);

	ks_os_poll_delay_msec(1);

	reg = RX8803_ADDR_YEAR;
    rtc_str = ((t->year/10)<<4) | (t->year%10);
	buffer[0] = reg;
	buffer[1] = rtc_str;
	ks_driver_i2c_master_send(0,RX8803_ADDR,buffer,2,50);


    return 0;
}

U8 RtcGetDate(RX8803_Date* t)
{

	U8 reg=0;
	U8 rtc_str;
	//bcd  -> oct格式
	reg = RX8803_ADDR_DAY;
	ks_driver_i2c_master_send(0,RX8803_ADDR,&reg,1,50);
	ks_driver_i2c_master_receive(0,RX8803_ADDR,&rtc_str,1,50);
	
    t->day = ((rtc_str>>4)*10) + (rtc_str& 0x0f);


	reg = RX8803_ADDR_MONTH;
    ks_driver_i2c_master_send(0,RX8803_ADDR,&reg,1,50);
	ks_driver_i2c_master_receive(0,RX8803_ADDR,&rtc_str,1,50);
    t->month = ((rtc_str>>4)*10) + (rtc_str& 0x0f);

	reg = RX8803_ADDR_YEAR;
	ks_driver_i2c_master_send(0,RX8803_ADDR,&reg,1,50);
	ks_driver_i2c_master_receive(0,RX8803_ADDR,&rtc_str,1,50);
    t->year = ((rtc_str>>4)*10) + (rtc_str& 0x0f);


    return 0;

}

U8 RtcSetTime(RX8803_Time* t)
{

	U8 reg=0;
    U8 rtc_str;
	U8 buffer[2];

	//oct -> bcd 格式
	reg = RX8803_ADDR_HOUR;
    rtc_str = ((t->hour/10)<<4) | (t->hour%10);
	buffer[0] = reg;
	buffer[1] = rtc_str;
	ks_driver_i2c_master_send(0,RX8803_ADDR,buffer,2,50);

	 
	ks_os_poll_delay_msec(1);
		
	reg = RX8803_ADDR_MINUTE;
	rtc_str = ((t->minute/10)<<4) | (t->minute%10);
	buffer[0] = reg;
	buffer[1] = rtc_str;

	ks_driver_i2c_master_send(0,RX8803_ADDR,buffer,2,50);

	ks_os_poll_delay_msec(1);

	reg = RX8803_ADDR_SECONDS;
    rtc_str = ((t->second/10)<<4) | (t->second%10);
	buffer[0] = reg;
	buffer[1] = rtc_str;

	ks_driver_i2c_master_send(0,RX8803_ADDR,buffer,2,50);

    return 0;
}

U8 RtcGetTime(RX8803_Time* t)
{
	U8 reg=0;
	U8 rtc_str;

	reg = RX8803_ADDR_SECONDS;
    ks_driver_i2c_master_send(0,RX8803_ADDR,&reg,1,50);
	ks_driver_i2c_master_receive(0,RX8803_ADDR,&rtc_str,1,50);
    t->second = ((rtc_str>>4)*10) + (rtc_str & 0x0f);

	reg = RX8803_ADDR_MINUTE;
    ks_driver_i2c_master_send(0,RX8803_ADDR,&reg,1,50);
	ks_driver_i2c_master_receive(0,RX8803_ADDR,&rtc_str,1,50);
    t->minute = ((rtc_str>>4)*10) + (rtc_str& 0x0f);

	reg = RX8803_ADDR_HOUR;
    ks_driver_i2c_master_send(0,RX8803_ADDR,&reg,1,50);
	ks_driver_i2c_master_receive(0,RX8803_ADDR,&rtc_str,1,50);
    t->hour =  ((rtc_str>>4)*10) + (rtc_str& 0x0f);


	
    return 0;

}



static int rtc_date_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int i, iret;
	uint32_t temp;
	RX8803_Date date;
 	memset(&date,0,sizeof(RX8803_Date));
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &temp);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   date.year =temp;

	   if(argc >= 3){
		   iret = ks_shell_str2uint(argv[2], &temp);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
	   }
	   date.month =temp;
	   if(argc >= 4){
		   iret = ks_shell_str2uint(argv[3], &temp);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
	   }
	   date.day =temp;

	   RtcSetDate(&date);
	   
	   ks_shell_printf(ctx->uart_id,"RtcSetDate: year:%d month:%d day:%d \r\n",date.year,date.month,date.day);
	}
	else
	{
		RtcGetDate(&date);
	
		ks_shell_printf(ctx->uart_id,"RtcGetDate: year:%d month:%d day:%d \r\n",date.year,date.month,date.day);
	}

	
    return 0;
}


static int rtc_time_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int i, iret;
	
	RX8803_Time time;
	uint32_t temp;
 	memset(&time,0,sizeof(RX8803_Time));
	if (argc >= 2 )
	{
		iret = ks_shell_str2uint(argv[1], &temp);
		if (iret != 0)
		{
		   return CMD_ERR_PARAMS_FORMAT;
		}
		
		time.hour = temp;
		if(argc >= 3){
		   iret = ks_shell_str2uint(argv[2], &temp);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
		
		time.minute = temp;
		if(argc >= 4){
		   iret = ks_shell_str2uint(argv[3], &temp);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
		
		time.second = temp;

		RtcSetTime(&time);

		ks_shell_printf(ctx->uart_id,"RtcSetTime: hour:%d minute:%d second:%d \r\n",time.hour,time.minute,time.second);
	}
	else
	{
		RtcGetTime(&time);
		
		ks_shell_printf(ctx->uart_id,"RtcSetTime: hour:%d minute:%d second:%d \r\n",time.hour,time.minute,time.second);
	}


    return 0;
}

static void timer_rtc_cb(void* arg){
	int uart_id = 0;
	uart_id = (*(int*)arg)<<8;

	ks_os_flag_set_bits(flag_handle_i2c_task, uart_id|1, SET_FLAG_MODE_OR);

}

static int rtc_tick_cmd(cmd_proc_t* ctx,int argc,  char **argv)
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
		if(timer_rtc == 0){
			ks_os_timer_coarse_create(&timer_rtc, "timer_rtc", timer_rtc_cb, (void*)&uart_id, 100, 1000, 0);
		}
		uart_id = ctx->uart_id;

		ks_os_timer_coarse_activate(timer_rtc);
	}else{
		if(timer_rtc != 0)
		ks_os_timer_coarse_deactivate(timer_rtc);
	}

	ks_shell_printf(ctx->uart_id,"rtc_tick_cmd : %d \r\n",enable);

    return 0;
}

static OSHandle timer_master_task;


static void timer_master_cb(void* arg){
	int uart_id = 0;
	uart_id = (*(int*)arg)<<8;

	ks_os_flag_set_bits(flag_handle_i2c_master_task, uart_id|1, SET_FLAG_MODE_OR);

}

void I2cMasterTickTask(void *p_arg)
{

	U8 reg=0;
	U8 len=0;
	uint32_t speed;

	U8* buffer = g_rx_buffer;
	
	speed =(uint32_t) p_arg;
	int iret ;

	i2c_config_t config;
	config.mode = IIC_MODE_MASTER;
	config.speed = speed;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	ks_os_timer_coarse_create(&timer_master_task, "timer_master_task", timer_master_cb, NULL, 100, 1000, 1);

	ks_shell_printf(0," master task  has start  speed %d  !!! \r\n",speed);
	
	while(1)
    {
	
		U32 status = ks_os_flag_wait(flag_handle_i2c_master_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
		reg++;
		len = reg;
		if(len== 0) len = 1 ;
		
		buffer[0] = reg;
		buffer[1] = len;
		iret = ks_driver_i2c_master_send(0,SLAVE_ADDR,buffer,2,500);
		if(iret == 0 ){
			ks_shell_printf(0,"master read: reg %x len %d \r\n",reg,len);
			iret = ks_driver_i2c_master_receive(0,SLAVE_ADDR,buffer,len,500);
			if(iret !=0){
				ks_shell_printf(0,"ks_driver_i2c_master_receive : %d \r\n",iret);
			}else{
			
				ks_shell_printf_dump_hex(0,buffer,len);
			}
		}else{
			ks_shell_printf(0,"ks_driver_i2c_master_send : %d \r\n",iret);
			ks_os_timer_coarse_deactivate(timer_master_task);
		}
	

    }
}

void I2cSlaveTickTask(void *p_arg)
{

	U8 reg=0;
	U8 len=0;
	uint32_t speed;

	U8* buffer = g_tx_buffer;

	int iret ;
	speed = (uint32_t)p_arg;

	i2c_config_t config;
	config.mode = IIC_MODE_SLAVE;
	config.speed = IIC_BUS_SPEED_HIGH;//设置最高，兼容所有的速率;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	ks_shell_printf(0,"slave  task  has start   speed %d !!! \r\n",IIC_BUS_SPEED_HIGH);
	while(1)
    {
		iret =  ks_driver_i2c_slave_receive(0,buffer,2,KS_OS_WAIT_FOREVER);
		if(iret == 0 ){
			reg = buffer[0];
			len = buffer[1];
			ks_shell_printf(0,"slave receive: reg %x len %d \r\n",reg,len);
			for (uint32_t i = reg; i <reg+len; i++) {  
	    		buffer[i-reg] = i&0xFF;
			}
	
			iret = ks_driver_i2c_slave_send(0,buffer,len,KS_OS_WAIT_FOREVER);
			if(iret!=0){
				ks_shell_printf(0,"ks_driver_i2c_slave_send : %d \r\n",iret);
			}else{
				ks_shell_printf(0,"slave send:\r\n");
				ks_shell_printf_dump_hex(0,buffer,len);
			}

		}else{
			ks_shell_printf(0,"ks_driver_i2c_slave_receive : %d \r\n",iret);
			break;
		}
    }
}


static int i2c_master_tick_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t speed;
	int iret ;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &speed);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}


	if(speed >4 || speed < 1 ){
	  	speed = 1;
	}


	if(thread_handle_i2c_master_task!=0){
		
		ks_shell_printf(ctx->uart_id,"i2c_master_task has create !!! \r\n");

		return -1;
	}
	
	ks_os_flag_create(&flag_handle_i2c_master_task, "i2c_master_flag");
	ks_os_thread_create(&thread_handle_i2c_master_task,			
						 "I2cMasterTickTask",					
						 I2cMasterTickTask,						
						 (void*)speed,
						 15,								
						 thread_stack_i2c_master_task,			
						 sizeof(thread_stack_i2c_master_task),	
						 0,
						 1
						 );


	return 0;

}

static int i2c_slave_tick_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t speed;
	int iret ;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &speed);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	 
	}


	if(speed >4 || speed < 1 ){
	  	speed = 1;
	}


	if(thread_handle_i2c_slave_task!=0){
		
		ks_shell_printf(ctx->uart_id,"i2c_master_slave_task has create !!! \r\n");

		return -1;
	}

	ks_os_thread_create(&thread_handle_i2c_slave_task,			
						 "I2cSlaveTickTask",					
						 I2cSlaveTickTask,						
						 (void*)speed,
						 15,								
						 thread_stack_i2c_slave_task,			
						 sizeof(thread_stack_i2c_slave_task),	
						 0,
						 1
						 );


	return 0;

}


static int i2c_master_send_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int iret;
	uint32_t  txlen,speed;
	U8 reg=0;
	
	U8* buffer = &g_tx_buffer[BUFFER_START_INDEX];
	
	for (uint32_t i = 0; i < TX_BUFFER_SIZE; i++) {
    	buffer[i] = i&0xFF;
	}

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &txlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(txlen > TX_BUFFER_SIZE ) txlen = TEST_BUFFER_LEN ;
	   
		if(argc > 2 ){
		   iret = ks_shell_str2uint(argv[2], &speed);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}else{
		txlen = TEST_BUFFER_LEN ;
	}

	if(speed > 4 || speed < 1 ){
	  	speed = 1;
	}

	i2c_config_t config;
	config.mode = IIC_MODE_MASTER;
	config.speed = speed;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	ks_shell_printf(ctx->uart_id,"master_send_cmd txlen %d speed %d buffer%64 %d \r\n",txlen,speed,(U32)buffer%64);
	
	int ret =ks_driver_i2c_master_send(0,SLAVE_ADDR,buffer,txlen,10000);
	//int ret =ks_driver_i2c_master_send(0,SLAVE_ADDR,buffer,txlen,3000);
	if(ret!=0){
		ks_shell_printf(0,"ks_driver_i2c_master_send : %d \r\n",ret);
	}else{
		ks_shell_printf(0,"master send:\r\n");
		ks_shell_printf_dump_hex(0,buffer,txlen);
	}


	return 0;

}



static int i2c_slave_send_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int iret;
	uint32_t  txlen,speed;
	
	U8* buffer = &g_tx_buffer[BUFFER_START_INDEX];
	
	for (uint32_t i = 0; i < TX_BUFFER_SIZE; i++) {
    	buffer[i] = i&0xFF;
	}

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &txlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(txlen > TX_BUFFER_SIZE) txlen = TEST_BUFFER_LEN ;
	   if(argc > 2 ){
		   iret = ks_shell_str2uint(argv[2], &speed);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}else{
		txlen = TEST_BUFFER_LEN ;
	}

	if(speed > 4 || speed < 1 ){
	  	speed = 1;
	}

	ks_shell_printf(ctx->uart_id,"slave_send_cmd txlen %d speed %d \r\n",txlen,speed);

	i2c_config_t config;
	config.mode = IIC_MODE_SLAVE;
	config.speed = speed;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	int ret = ks_driver_i2c_slave_send(0,buffer,txlen,15000);
	//int ret = ks_driver_i2c_slave_send(0,buffer,txlen,1000);
	if(ret!=0){
		ks_shell_printf(0,"ks_driver_i2c_slave_send : %d \r\n",ret);
	}else{
		ks_shell_printf(0,"slave send:\r\n");
		ks_shell_printf_dump_hex(0,buffer,txlen);
	}

	return 0;
}

static int i2c_master_receive_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	
	int iret;
	uint32_t  rxlen,speed;

	U8* buffer = &g_rx_buffer[BUFFER_START_INDEX];

	memset(buffer,0,RX_BUFFER_SIZE);
	
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &rxlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(rxlen > RX_BUFFER_SIZE ) rxlen = TEST_BUFFER_LEN ;
	   if(argc > 2 ){
		   iret = ks_shell_str2uint(argv[2], &speed);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}else{
		rxlen = TEST_BUFFER_LEN ;
	}

	if(speed > 4 || speed < 1 ){
	  	speed = 1;
	}
	i2c_config_t config;
	config.mode = IIC_MODE_MASTER;
	config.speed = speed;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	ks_shell_printf(ctx->uart_id,"master start receive rxlen %d speed %d buffer%64 %d \r\n",rxlen,speed,(U32)buffer%64);

	int ret = ks_driver_i2c_master_receive(0,SLAVE_ADDR,buffer,rxlen,15000);
	

	ks_shell_printf(ctx->uart_id,"master_receive_cmd  receive ret %d \r\n",ret);

	if(ret == 0){
		ks_shell_printf_dump_hex(ctx->uart_id,buffer,rxlen);
	}


	return 0;

}

static int i2c_slave_receive_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int iret;
	uint32_t  rxlen,speed;

	memset(g_rx_buffer,0,RX_BUFFER_SIZE);
	
	U8* buffer = &g_rx_buffer[BUFFER_START_INDEX];

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &rxlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(rxlen > RX_BUFFER_SIZE ) rxlen = TEST_BUFFER_LEN ;
	   
	   if(argc > 2 ){
		   iret = ks_shell_str2uint(argv[2], &speed);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}else{
		rxlen = TEST_BUFFER_LEN ;
	}


	if(speed > 4 || speed < 1 ){
	  	speed = IIC_BUS_SPEED_HIGH;
	}

	i2c_config_t config;
	config.mode = IIC_MODE_SLAVE;
	config.speed = speed ;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);


	ks_shell_printf(ctx->uart_id,"slave start receive rxlen %d speed %d \r\n",rxlen,speed);
	//slave rx during 0-15 s
	int ret = ks_driver_i2c_slave_receive(0,buffer,rxlen,150000);

	ks_shell_printf(ctx->uart_id,"slave_receive_cmd  receive ret %d \r\n",ret);

	if(ret == 0){
		ks_shell_printf_dump_hex(ctx->uart_id,buffer,rxlen);
	}


	return 0;
}

void master_tx_timer_loop_cb(void *p_arg){
	int curent_count = *(int*) p_arg;
	static int last_count = 0;
	if(curent_count!=0 ){
		if(curent_count > last_count ){
			ks_shell_printf(0,"master send: %d \r\n",curent_count);
			ks_shell_printf(0,"\r\n");
			last_count = curent_count;
		}
	}

}

void I2cMasterTxLoopTask(void *p_arg)
{

	int  len = 1024;
	uint32_t speed;
	int count = 0;
	
	U8* buffer = &g_tx_buffer[BUFFER_START_INDEX];

	
	for (uint32_t i = 0; i < TX_BUFFER_SIZE ; i++) {
    	buffer[i] = i&0xFF;
	}

	int iret ;
	speed = (uint32_t)p_arg&0xFFFF;
	len = (uint32_t)p_arg >>16&0xFFFF;

	i2c_config_t config;
	config.mode = IIC_MODE_MASTER;
	config.speed = speed;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	ks_shell_printf(0,"master tx task  has start len %d  speed %d  buffer%64 %x !!! \r\n",len,speed,(U32)buffer%64);
	
	U32 ret = ks_os_timer_coarse_create(&timer_loop, "timer_loop", master_tx_timer_loop_cb, &count, 100, 1000, 1);
	
	while(1)
    {
		iret = ks_driver_i2c_master_send(0,SLAVE_ADDR,buffer,len,1000);
		if(iret!=0){
			ks_shell_printf(0,"ks_driver_i2c_master_send error: %d \r\n",iret);
			ks_os_timer_coarse_deactivate(timer_loop);
			break;
		}else{
			/*if(count%100 == 0){
				ks_shell_printf(0,"master send: %d \r\n",count);
				//ks_shell_printf_dump_hex(0,buffer,len);
				ks_shell_printf(0," \r\n");
			}*/
			count++;
		}
		
		//延时 保证slave 接收数据后判断数据是否正确 进入 recieve 状态，master 才能发送 数据越长，延时越大 长度2048 slave 比较耗时28 us
		ks_os_poll_delay_usec(1000);

    }
}


static int i2c_master_tx_loop_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t speed,txlen;
	int iret ;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &txlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(txlen > TX_BUFFER_SIZE ) txlen = TEST_BUFFER_LEN ;
	   if(argc > 2 ){
		   iret = ks_shell_str2uint(argv[2], &speed);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}else{
		txlen = TEST_BUFFER_LEN ;
	}

	if(speed > 4 || speed < 1 ){
	  	speed = 1;
	}

	if(thread_handle_i2c_master_task!=0){
		
		ks_shell_printf(ctx->uart_id,"i2c_master_task has create !!! \r\n");

		return -1;
	}
	
	ks_os_flag_create(&flag_handle_i2c_master_task, "i2c_master_flag");
	ks_os_thread_create(&thread_handle_i2c_master_task,			
						 "I2cMasterTxLoopTask",
						 I2cMasterTxLoopTask,						
						 (void*)(txlen<<16|speed),
						 15,								
						 thread_stack_i2c_master_task,			
						 sizeof(thread_stack_i2c_master_task),	
						 0,
						 1
						 );


	return 0;

}


void master_rx_timer_loop_cb(void *p_arg){
	int curent_count = *(int*) p_arg;
	static int last_count = 0;
	if(curent_count!=0 ){
		if(curent_count > last_count ){
			ks_shell_printf(0,"master receive: %d \r\n",curent_count);
			ks_shell_printf(0,"\r\n");
			last_count = curent_count;
		}
	}

}

void I2cMasterReadTask(void *p_arg)
{

	int len = 1024;
	uint32_t speed;
	int count = 0;

	U8* buffer = &g_rx_buffer[BUFFER_START_INDEX];

	speed = (uint32_t)p_arg&0xFFFF;
	len = (uint32_t)p_arg >>16&0xFFFF;
	memset(buffer,0,len);

	int iret ;

	i2c_config_t config;
	config.mode = IIC_MODE_MASTER;
	config.speed = speed;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	ks_shell_printf(0,"master rx task  has start len %d speed %d  buffer%64 %x !!! \r\n",len,speed,(U32)buffer%64);
	
	U32 ret = ks_os_timer_coarse_create(&timer_loop, "timer_loop", master_rx_timer_loop_cb, &count, 100, 1000, 1);
	while(1)
    {
		iret = ks_driver_i2c_master_receive(0,SLAVE_ADDR,buffer,len,1000);
		if(iret ==0){

			for (int i = 0; i < len; i++) {
				if (buffer[i] != (U8)(i&0xFF) ) {
					ks_shell_printf(0,"data error  i %d dst %d src %d \r\n",i,buffer[i],i&0xFF);
					ks_os_thread_sleep_msec(1000);
					return;
		        }
			}
			/*if(count%100 == 0){
				ks_shell_printf(0,"master receive: %d\r\n",count);
				//ks_shell_printf_dump_hex(0,buffer,len);
				ks_shell_printf(0," \r\n");
			}*/
			memset(buffer,0,len);
			
			count++;
		}else{
			ks_shell_printf(0," ks_driver_i2c_master_receive  %d  !!! \r\n",iret);
			ks_os_timer_coarse_deactivate(timer_loop);
			return;

		}
		//延时 保证slave 进入时 send  状态后， master 才能发起读取
		ks_os_poll_delay_usec(10);

    }
}

static int i2c_master_rx_loop_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{

	uint32_t speed,rxlen;
	int iret ;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &rxlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(rxlen > RX_BUFFER_SIZE  ) rxlen = TEST_BUFFER_LEN ;
	   if(argc > 2 ){
		   iret = ks_shell_str2uint(argv[2], &speed);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}else{
		rxlen = TEST_BUFFER_LEN ;
	}

	if(speed > 4 || speed < 1 ){
		speed = 1;
	}



	if(thread_handle_i2c_master_task!=0){
		
		ks_shell_printf(ctx->uart_id,"i2c_master_task has create !!! \r\n");

		return -1;
	}
	
	ks_os_flag_create(&flag_handle_i2c_master_task, "i2c_master_flag");
	ks_os_thread_create(&thread_handle_i2c_master_task,			
						 "I2cMasterReadTask",
						 I2cMasterReadTask,						
					 	(void*)(rxlen<<16|speed),
						 15,								
						 thread_stack_i2c_master_task,			
						 sizeof(thread_stack_i2c_master_task),	
						 0,
						 1
						 );


	return 0;

}



void slave_rx_timer_loop_cb(void *p_arg){

	int curent_count = *(int*) p_arg;
	static int last_count = 0;
	if(curent_count!=0 ){
		if(curent_count > last_count ){
			ks_shell_printf(0,"slave receive: %d \r\n",curent_count);
			ks_shell_printf(0,"\r\n");
			last_count = curent_count;
		}
	}

}

void I2cSlaveRxLoopTask(void *p_arg)
{

	int  len = 1024;
	uint32_t speed;
	int count = 0;
	U8* buffer_ping_pang[2];

	 buffer_ping_pang[0]= &g_rx_buffer[BUFFER_START_INDEX];
	 buffer_ping_pang[1]= &g_tx_buffer[BUFFER_START_INDEX];
	 
	//U8* buffer = &g_rx_buffer[1];
	//memset(buffer,0,len);

	
	float us;
	uint64_t start_ticks = 0;
	uint64_t end_ticks = 0;

	int iret ;
	speed = (uint32_t)p_arg&0xFFFF;
	len = (uint32_t)p_arg >>16&0xFFFF;
	
	ks_os_thread_vfp_enable();


	i2c_config_t config;
	config.mode = IIC_MODE_SLAVE;
	config.speed = speed;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	ks_shell_printf(0,"slave  rx task  has start len %d   speed %d  buffer%64 %x !!! \r\n",len,speed,(U32)buffer_ping_pang[0]%64);
	
	U32 ret = ks_os_timer_coarse_create(&timer_loop, "timer_loop", slave_rx_timer_loop_cb, &count, 100, 1000, 1);

	while(1)
    {
		if(s_i2c_rx_check.check_ret == 0){
			
			iret = ks_driver_i2c_slave_receive(0,buffer_ping_pang[count%2],len,KS_OS_WAIT_FOREVER);
			if(iret ==0){
				
				if(s_i2c_rx_check.check_req == 0){
					s_i2c_rx_check.check_req =1;
					s_i2c_rx_check.current_rx_buffer = buffer_ping_pang[count%2];
					ks_os_flag_set_bits(flag_handle_i2c_check_task, 1, SET_FLAG_MODE_OR);
				}
				
				count++;
			}
		}else{
			ks_os_thread_sleep_msec(1000);
			ks_os_timer_coarse_deactivate(timer_loop);
		}

    }
}

void I2cSlaveRxLoopCheckTask(void *p_arg)
{
	int  len = 1024;
	uint32_t speed;
	int count = 0;
	float us;
	uint64_t start_ticks = 0;
	uint64_t end_ticks = 0;
	U8* buffer ;

	int iret ;
	speed = (uint32_t)p_arg&0xFFFF;
	len = (uint32_t)p_arg >>16&0xFFFF;
	
	ks_os_thread_vfp_enable();
	
	ks_shell_printf(0,"slave rx  check task  has start len %d  speed %d !!! \r\n",len,speed);


	while(1)
    {
	
		U32 status = ks_os_flag_wait(flag_handle_i2c_check_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);

		if(s_i2c_rx_check.check_req == 1){

			//start_ticks = ks_os_get_free_time();
			
			buffer= s_i2c_rx_check.current_rx_buffer;

			s_i2c_rx_check.check_ret = 0;
			
			for (int i = 0; i < len; i++) {
				if (buffer[i] != (U8)(i&0xFF) ) {
					s_i2c_rx_check.check_ret = 1;
					ks_shell_printf(0,"data error  i %d dst %d src %d \r\n",i,buffer[i],i&0xFF);
					break;
		        }
			}

			memset(buffer,0,len);
			s_i2c_rx_check.check_req = 0;
			//end_ticks = ks_os_get_free_time();
			//us = 1000000.0f * (end_ticks - start_ticks) / ks_os_get_sys_clock() ;
			//ks_shell_printf(0,"%.2f\r\n",us);

		}

    }
}

static int i2c_slave_rx_loop_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t speed,rxlen;
	int iret ;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &rxlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(rxlen > RX_BUFFER_SIZE ) rxlen = TEST_BUFFER_LEN ;
	   if(argc > 2 ){
		   iret = ks_shell_str2uint(argv[2], &speed);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}else{
		rxlen = TEST_BUFFER_LEN ;
	}

	if(speed > 4 || speed < 1 ){
	  	speed = 1;
	}


	if(thread_handle_i2c_slave_task!=0){
		
		ks_shell_printf(ctx->uart_id,"i2c_slave_task has create !!! \r\n");

		return -1;
	}

	ks_os_thread_create(&thread_handle_i2c_slave_task,			
						 "I2cSlaveRxLoopTask",
						 I2cSlaveRxLoopTask,
						 (void*)(rxlen<<16|speed),
						 15,								
						 thread_stack_i2c_slave_task,			
						 sizeof(thread_stack_i2c_slave_task),	
						 0,
						 1
						 );
	ks_os_flag_create(&flag_handle_i2c_check_task, "i2c_check_flag");
	ks_os_thread_create(&thread_handle_i2c_slave_check_task,			
						 "I2cSlaveRxLoopCheckTask",
						 I2cSlaveRxLoopCheckTask,
						 (void*)(rxlen<<16|speed),
						 16,								
						 thread_stack_i2c_slave_check_task,			
						 sizeof(thread_stack_i2c_slave_check_task),	
						 0,
						 1
						 );


	return 0;

}

void slave_tx_timer_loop_cb(void *p_arg){
	int curent_count = *(int*) p_arg;
	static int last_count = 0;
	if(curent_count!=0 ){
		if(curent_count > last_count ){
			ks_shell_printf(0,"slave send: %d \r\n",curent_count);
			ks_shell_printf(0,"\r\n");
			last_count = curent_count;
		}
	}

}

void I2cSlaveTxLoopTask(void *p_arg)
{

	int  len = 1024;
	uint32_t speed;
	int count = 0;
	
	U8* buffer = &g_tx_buffer[BUFFER_START_INDEX];
	for (uint32_t i = 0; i < TX_BUFFER_SIZE ; i++) {
    	buffer[i] = i&0xFF;
	}

	int iret ;
	speed = (uint32_t)p_arg&0xFFFF;
	len = (uint32_t)p_arg >>16&0xFFFF;


	i2c_config_t config;
	config.mode = IIC_MODE_SLAVE;
	config.speed = speed;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);

	ks_shell_printf(0,"slave tx task  has start  len %d  speed %d  buffer%64 %x !!! \r\n",len,speed,(U32)buffer%64);
	
	U32 ret = ks_os_timer_coarse_create(&timer_loop, "timer_loop", slave_tx_timer_loop_cb, &count, 100, 1000, 1);
	
	while(1)
    {
		iret = ks_driver_i2c_slave_send(0,buffer,len,KS_OS_WAIT_FOREVER);
		if(iret!=0){
			ks_shell_printf(0,"ks_driver_i2c_slave_send : %d \r\n",iret);
			ks_os_timer_coarse_deactivate(timer_loop);
		}else{
			/*if(count%100 == 0){
				ks_shell_printf(0,"slave send: %d \r\n",count);
				//ks_shell_printf_dump_hex(0,buffer,len);
				ks_shell_printf(0," \r\n");
			}*/
			count++;
		}

    }
}

static int i2c_slave_tx_loop_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	uint32_t speed,txlen;
	int iret ;
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &txlen);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }

	   if(txlen > TX_BUFFER_SIZE ) txlen = TEST_BUFFER_LEN ;
	   if(argc > 2 ){
		   iret = ks_shell_str2uint(argv[2], &speed);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}else{
		txlen = TEST_BUFFER_LEN ;
	}

	if(speed > 4 || speed < 1 ){
	  	speed = 1;
	}


	if(thread_handle_i2c_slave_task!=0){
		
		ks_shell_printf(ctx->uart_id,"I2cSlaveTxLoopTask has create !!! \r\n");

		return -1;
	}

	ks_os_thread_create(&thread_handle_i2c_slave_task,			
						 "I2cSlaveTxLoopTask",
						 I2cSlaveTxLoopTask,
						 (void*)(txlen<<16|speed),
						 15,								
						 thread_stack_i2c_slave_task,			
						 sizeof(thread_stack_i2c_slave_task),	
						 0,
						 1
						 );


	return 0;

}


static cmd_proc_t i2c_demo_cmds[] = {
    {.cmd = "date", .fn = rtc_date_cmd, .help = "data <year> <month> <day> "},
    {.cmd = "time", .fn = rtc_time_cmd, .help = "time <hour> <minute> <second>"},
    {.cmd = "rtctick", .fn = rtc_tick_cmd, .help = "rtctick <enable/disable> "},

	{.cmd = "m_tx", .fn = i2c_master_send_cmd, .help = "m_tx  <txlen> <speed>"},
	{.cmd = "m_rx", .fn = i2c_master_receive_cmd, .help = "m_rx <rxlen> <speed>"},
	{.cmd = "m_tl", .fn = i2c_master_tx_loop_cmd, .help = "m_tl <txlen> <speed> "},
	{.cmd = "m_rl", .fn = i2c_master_rx_loop_cmd, .help = "m_rl <rxlen> <speed>"},
    {.cmd = "s_tx", .fn = i2c_slave_send_cmd, .help = "s_tx <txlen> <speed>"},
    {.cmd = "s_rx", .fn = i2c_slave_receive_cmd, .help = "s_rx <rxlen> <speed>"},
    {.cmd = "s_rl", .fn = i2c_slave_rx_loop_cmd, .help = "s_rl <rxlen> <speed>"},
	{.cmd = "s_tl", .fn = i2c_slave_tx_loop_cmd, .help = "s_tl <txlen> <speed> "},

	
    {.cmd = "s_tick", .fn = i2c_slave_tick_cmd , .help = "s_tick <speed>"},
    {.cmd = "m_tick", .fn = i2c_master_tick_cmd , .help = "m_tick <speed>"},
      

};

void I2cDemoTask(void *p_arg)
{
	RX8803_Date rtc_date;
	RX8803_Time rtc_time;
	U32 uart_id;
	ks_os_thread_vfp_enable();

	ks_shell_add_cmds(i2c_demo_cmds, sizeof(i2c_demo_cmds) / sizeof(cmd_proc_t));

	i2c_config_t config;
	config.mode = IIC_MODE_MASTER;
	config.speed = IIC_BUS_SPEED_STANDARD;
	config.addr_mode = IIC_ADDRESS_7BIT;
	config.slave_addr = SLAVE_ADDR;

	ks_driver_i2c_config(0,&config);


	while(1)
    {
    	U32 status = ks_os_flag_wait(flag_handle_i2c_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
		uart_id = status>>8;


		RtcGetDate(&rtc_date);
		RtcGetTime(&rtc_time);
		
		ks_shell_printf(uart_id,"date: %d:%d:%d  time: %d:%d:%d \r\n",rtc_date.year,rtc_date.month,rtc_date.day,rtc_time.hour,rtc_time.minute,rtc_time.second);

    }
}



void I2cDemoInit(void)
{

	ks_os_flag_create(&flag_handle_i2c_task, "i2c_demo_flag");
	ks_os_thread_create(&thread_handle_i2c_task,			
							 "i2c_demo_task",					
							 I2cDemoTask,						
							 0,								
							 15,								
							 thread_stack_i2c_task,			
							 sizeof(thread_stack_i2c_task),	
							 0,
							 1
							 );

}




