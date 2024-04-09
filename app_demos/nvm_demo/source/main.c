
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  main.h
 *
 * @brief SDK 应用层初始化入口，main主任务，创建nvm demo等
 */

#include <string.h>
#include "ks_include.h"
#include "nvm/nvm_demo.h"
#include "ks_uart.h"
#include "ks_shell.h"
#include "ks_exception.h"


/// SDK应用层初始化入口
void ks_app_init()
{
    /// 创建NVM测试线程
    NvmDemoInit();
}



/// components 中间件初始化入口
void ks_components_init()
{
	/// 创建SHELL 串口交互处理线程
	ks_shell_init(PROJECT_NAME);
	///　使能对应uart shell 交互
	ks_shell_uart_enable(0,1);

	ks_exception_project_name_set(PROJECT_NAME);
	ks_exception_output_uart_set(0);
}


/// SDK驱动层初始化入口
void ks_drivers_init()
{
    ks_driver_uart_init(0, 460800, 0);
    ks_driver_uart_init(1, 460800, 0);
    ks_driver_uart_init(2, 460800, 0);
    ks_driver_uart_init(3, 460800, 0);
}



