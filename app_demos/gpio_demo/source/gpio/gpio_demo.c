
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  gpio.c
 *
 * @brief  gpio 演示实现　
 */

#include <gpio_demo.h>
#include <stdlib.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "ks_shell.h"
#include "ks_gpio.h"




#define  STACK_SIZE_GPIO_TASK          512
static S32 thread_stack_gpio_task[STACK_SIZE_GPIO_TASK];
static OSHandle flag_handle_gpio_task;
static OSHandle thread_handle_gpio_task;



void gpio_pin_level_printf(int gpioid, uint32_t input)
{
    uint8_t temp[33] = {0};  
    int i = 0;
    while(input)
    {
        temp[i] = input % 2;   
        input = (uint32_t)input / 2;  
        i++; 
    }
	
    for(i--; i>=0; i--)  
    {	if(gpioid == i || gpioid== -1)
        ks_shell_printf(0,"gpio%d: %d\r\n",i,temp[i]);
    }

}

void gpio_isr_handler(void* arg)
{														 
	U32 status;
	U8 gpio;
	//读取中断状态以便确定是哪个GPIO发生中断
	status = ks_driver_gpio_interrupt_status();
	
	//先屏蔽对应的中断
	ks_driver_gpio_interrupt_mask_set(status);

	//清除中断,该操作同时清除中断状态寄存器GPIOx_INTSTATUS 和原始中断状态寄存器GPIOx_RAW_INTSTATUS
	// 注意　ks_driver_gpio_interrupt_clear　和　ks_driver_gpio_interrupt_clear_all　参数的区别
	for(int i= 0 ;i<32;i++){
		if(status&(1<<i)){
			gpio = i;
			ks_driver_gpio_interrupt_clear(gpio);
		}
	}
	//ks_driver_gpio_interrupt_clear_all(status);

	ks_os_flag_set_bits(flag_handle_gpio_task, status, SET_FLAG_MODE_OR);
	
	ks_driver_gpio_interrupt_mask_set(0);

}


/**
 * 
 * @param ctx [in]
 *      命令输入结构体上下文指针
 * @param argc [in]
 *      输入参数个数
  * @param argc [in]
 *      输入参数组指针
 * @return　
 *      
 */
static int gpio_irq_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int i, iret;
	uint32_t gpio_id ;
	uint32_t irq_mode;
	uint32_t len;

	len = 0;

	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &gpio_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	   	iret = ks_shell_str2uint(argv[2], &irq_mode);
		if (iret != 0)
		{
			return CMD_ERR_PARAMS_FORMAT;
		}
	}
	else
	{
	  	gpio_id = 9;
		irq_mode = 1;
	}

	//复用功能切换gpio
	ks_driver_gpio_init(gpio_id);
	
	//设置GPIO 方向输入
	ks_driver_gpio_set_dir(gpio_id,GPIO_INPUT);
	ks_driver_gpio_set_pull_dir(gpio_id,GPIO_PULL_UP);
	
	ks_driver_gpio_set_pull_enable(gpio_id,1);
	
	//设置中断触发模式　触发极性

	ks_driver_gpio_interrupt_config(gpio_id,TRIGGER_TYPE_EDGE,irq_mode);
	ks_shell_printf(ctx->uart_id,"irq_mode %s   \r\n",irq_mode>0?"TRIGGER_RISING_EDGE":"TRIGGER_FALLING_EDGE");

	ks_driver_gpio_irq_create(gpio_isr_handler,(void*)ctx->uart_id);

	ks_driver_gpio_interrupt_clear(gpio_id);

	//解除屏蔽中断
	ks_driver_gpio_interrupt_mask(gpio_id,0);
	
	ks_driver_gpio_interrupt_enable(gpio_id, 1);

	ks_driver_gpio_irq_enable();
	
	ks_shell_printf(ctx->uart_id,"gpio_irq : %d \r\n",gpio_id);

    return 0;
}



/**
 * @param ctx [in]
 *     命令输入结构体上下文指针
 * @param argc [in]
 *      输入参数个数
  * @param argc [in]
 *      输入参数组指针
 * @return　
 *      
 */
static int gpio_input_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
	int i, iret;
	uint32_t gpio_id ;
	uint32_t output_level;
	uint32_t len;

	len = 0;
	
	//input gpio 10 
	if (argc >= 2 )
	{
	   iret = ks_shell_str2uint(argv[1], &gpio_id);
	   if (iret != 0)
	   {
		   return CMD_ERR_PARAMS_FORMAT;
	   }
	}
	else
	{
		gpio_id = 10;
	}

	
	//复用功能切换 gpio
	ks_driver_gpio_init(gpio_id);
	
	//设置GPIO 方向输入
	ks_driver_gpio_set_dir(gpio_id,GPIO_INPUT);
	
	U32 gpio_status = ks_driver_gpio_status();

	gpio_pin_level_printf(gpio_id,gpio_status);

    return 0;
}



/**
 * GPIO输出命令实现　　
 * @param ctx [in]
 *      命令上下文指针
 * @param argc [in]
 *      输入参数个数
  * @param argc [in]
 *      输入参数组指针
 * @return　
 *      
 */

static int gpio_output_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int i, iret;
    uint32_t gpio_id;
	uint32_t output_level;
    uint32_t len;

    len = 0;

    if (argc > 2 )
    {
		iret = ks_shell_str2uint(argv[1], &gpio_id);
		if (iret != 0)
		{
		 	return CMD_ERR_PARAMS_FORMAT;
		}
		iret = ks_shell_str2uint(argv[2], &output_level);
		if (iret != 0)
		{
			return CMD_ERR_PARAMS_FORMAT;
		}
    }
    else
    {
		gpio_id = 12;
		output_level = 1;
    }


	//复用功能切换 gpio
	ks_driver_gpio_init(gpio_id);
	
	//设置GPIO 方向输出
	ks_driver_gpio_set_dir(gpio_id,GPIO_OUTPUT);
	
	//设置gpio 输出电平
	ks_driver_gpio_set_output_level(gpio_id,output_level);

	ks_shell_printf(ctx->uart_id, "gpio%d	output_level %d  \r\n",gpio_id, output_level);

    return 0;
}

/**
 * @brief       GPIO 上下拉配置命令
 *
 * @param ctx   命令上下文指针
 * @param argc  输入参数个数
 * @param argv  输入参数组指针
 * @return
 */
static int gpio_pull_cmd(cmd_proc_t* ctx,int argc,char **argv)
{
    int iret;

    uint32_t gpio_id = 14;
    uint32_t enable = 0;
    uint32_t dir = 0;

    if (argc > 3 )
    {
        iret = ks_shell_str2uint(argv[1], &gpio_id);
        if (iret != 0)
        {
           return CMD_ERR_PARAMS_FORMAT;
        }

        iret = ks_shell_str2uint(argv[2], &enable);
        if (iret != 0)
        {
           return CMD_ERR_PARAMS_FORMAT;
        }

        iret = ks_shell_str2uint(argv[3], &dir);
        if (iret != 0)
        {
           return CMD_ERR_PARAMS_FORMAT;
        }
    }
    else{
        ks_shell_printf(ctx->uart_id, "%s <gpio-id> <pull-enable> <pull-dir,0:down,1:up>\r\n", gpio_id, enable, dir);
        return 0;
    }

    //复用功能切换GPIO
    ks_driver_gpio_init(gpio_id);

    //设置GPIO 方向输入
    ks_driver_gpio_set_dir(gpio_id, GPIO_INPUT);

    //设置上下拉配置
    ks_driver_gpio_set_pull_enable(gpio_id, enable);
    ks_driver_gpio_set_pull_dir(gpio_id, dir);

    //屏蔽中断，仅测试GPIO上下拉
    ks_driver_gpio_interrupt_enable(gpio_id, 0);

    return 0;
}


 
static cmd_proc_t gpio_demo_cmds[] = {
    {.cmd = "gpio_input" , .fn = gpio_input_cmd , .help = "gpio_input <gpio>"                                       },
    {.cmd = "gpio_output", .fn = gpio_output_cmd, .help = "gpio_output <gpio_id> <output_level>"                    },
    {.cmd = "gpio_irq"   , .fn = gpio_irq_cmd   , .help = "gpio_irq <gpio> <irq_mode>"                              },
    {.cmd = "gpio_pull"  , .fn = gpio_pull_cmd  , .help = "gpio_pull <gpio-id> <pull-enable> <pull-dir,0:down,1:up>"},
};

void GpioDemoTask(void *p_arg)
{
	ks_os_thread_vfp_enable();

	ks_shell_add_cmds(gpio_demo_cmds, sizeof(gpio_demo_cmds) / sizeof(cmd_proc_t));

	while(1)
    {
    	U32 status = ks_os_flag_wait(flag_handle_gpio_task, -1, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
	
		for(int i= 0 ;i<32;i++){
			if(status&(1<<i)){
				ks_shell_printf(0,"gpio%d: has interrupt!!! \r\n",i);
			}
		}

    }
}


void GpioDemoInit(void)
{

	ks_os_flag_create(&flag_handle_gpio_task, "gpio_demo_task");
	ks_os_thread_create(&thread_handle_gpio_task,			
							 "gpio_demo_task",					
							 GpioDemoTask,						
							 0,								
							 15,								
							 thread_stack_gpio_task,			
							 sizeof(thread_stack_gpio_task),	
							 0,
							 1
							 );

	
}




