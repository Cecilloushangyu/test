
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  main.c
 *
 * @brief SDK 应用层初始化入口，main主任务，创建mem demo等
 */

#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "mem_demo.h"
#include "ks_mem.h"
#include "ks_shell.h"
#include "ks_exception.h"
#include "ks_mem.h"

/// SDK应用层初始化入口 
void ks_app_init()
{
	/// 创建Mem Demo线程
	MemDemoInit();					
}

/// components 中间件初始化入口
void ks_components_init()
{

	/// 创建SHELL 串口交互处理线程
	ks_shell_init(PROJECT_NAME);
	///　使能对应uart shell 交互
	ks_shell_uart_enable(0,1);
	ks_shell_uart_enable(1,1);
	ks_shell_print_version(0);

	ks_exception_project_name_set(PROJECT_NAME);
	ks_exception_output_uart_set(0);
	
	ks_mem_init();


}

/// SDK驱动层初始化入口 
void ks_drivers_init()
{

    ks_driver_uart_init(0,115200,0);
	ks_driver_uart_init(1,115200,0);

}



