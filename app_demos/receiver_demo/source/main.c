
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  main.c
 *
 * @brief SDK 应用层初始化入口，启动PROTOCOL中间件
 */

#include "middleware/protocol/ks_protocol.h"
#include "middleware/nvm/ks_nvm.h"
#include "ks_exception.h"
#include "ks_uart.h"
#include "ks_flash.h"
#include "ks_bb.h"

#define PROJECT_NAME "delos_receiver_demo"

/// SDK应用层初始化入口
void ks_app_init()
{
    PARTITION_HANDLE handle;
    ks_nvm_create_partition(DEFAULT_FLASH_START_ADDR, 512, 1024, &handle);
    ks_protocol_init();
    PROTOCOL_CONFIG config;
    ks_protocol_config_get(&config);
    config.rx_cfg[0] = RX_L1;
    config.rx_cfg[1] = RX_L2_L5;
    config.rx_cfg[2] = RX_B3_L6;
    config.rx_cfg[3] = RX_NULL;
    config.flash_partition_handle = handle;
    ks_protocol_config_set(&config);
    ks_protocol_run();
}

/// components 中间件初始化入口
void ks_components_init()
{
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

