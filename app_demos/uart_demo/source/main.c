
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  main.c
 *
 * @brief SDK 应用层初始化入口，main主任务，创建uart demo等
 */

#include <uart_demo.h>
#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "shell_api.h"
#include "ks_taskdef.h"
#include "ks_exception.h"




// SDK应用层初始化入口 
void ks_app_init()
{

	// 创建Uart Demo线程
	UartDemoInit();
}


// components 中间件初始化入口
void ks_components_init()
{

	// 创建SHELL 串口交互处理线程
	ks_shell_init(PROJECT_NAME);
	//　使能对应uart shell 交互
	ks_shell_uart_enable(0,1);
	//ks_shell_uart_enable(1,1);
	//ks_shell_uart_enable(2,1);
	//ks_shell_uart_enable(3,1);
	ks_shell_print_version(0);
	ks_shell_print_version(1);
	ks_shell_print_version(2);
	ks_shell_print_version(3);

	//异常输出默认配置
	ks_exception_project_name_set(PROJECT_NAME);
	ks_exception_output_uart_set(0);


}


/// SDK驱动层初始化入口 
void ks_drivers_init()
{
    ks_driver_uart_init(0,115200,0);
    //ks_driver_uart_init(0,115200,TX_DMA_ENABLE);
	ks_driver_uart_init(1,115200,RX_DMA_ENABLE);
	//ks_driver_uart_init(1,115200,0);
	ks_driver_uart_init(2,115200,TX_DMA_ENABLE);
	//ks_driver_uart_init(2,115200,0);
	ks_driver_uart_init(3,115200,0);


}



