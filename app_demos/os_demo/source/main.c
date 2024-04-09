
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  main.h
 *
 * @brief SDK 应用层初始化入口，main主任务，创建app demo等
 */

#include <string.h>
#include "ks_include.h"
#include "os/os_demo.h"
#include "ks_uart.h"



/// components 中间件初始化入口
void ks_components_init()
{
	ks_exception_project_name_set(PROJECT_NAME);
	ks_exception_output_uart_set(0);
}


/// SDK应用层初始化入口
void ks_app_init()
{
    /// 运行OS Demo
    RunOsDemo();

}

/// SDK驱动层初始化入口
void ks_drivers_init()
{
    ks_driver_uart_init(0, 460800, 0);
    ks_driver_uart_init(1, 460800, 0);
    ks_driver_uart_init(2, 460800, 0);
    ks_driver_uart_init(3, 460800, 0);
}



