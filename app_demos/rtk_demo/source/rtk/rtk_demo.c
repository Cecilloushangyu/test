/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  rtk_demo.c
 *
 * @brief  RTK使用示例
 */

#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "middleware/rtk/ks_rtk.h"
#include "middleware/nmea/ks_nmea.h"
#include "pvt_type.h"

#define _STACK_SIZE_RTK_PRINTING        512
static S32 thread_stack_rtk_printing[_STACK_SIZE_RTK_PRINTING];
static OSHandle thread_handle_rtk_printing;
static OSHandle flag_handle_result_printing;
#define PRINT_RESULTS 1

static char print_buffer[8192];

// pvt缓存结果
PvtResult g_pvt_result;

typedef union
{
    DOUBLE d_data;
    U32 i_data[2];
} DOUBLE_INT_UNION;

static S32 IsNaN(DOUBLE value)
{
    DOUBLE_INT_UNION data;
    data.d_data = value;
    if ((data.i_data[1] & 0x7ff00000) == 0x7ff00000)
    {
        if ((data.i_data[1] & 0x000fffff) || (data.i_data[0]))
            return 1;
        else
            return 0;
    }
    return 0;
}

static S32 IsInf(DOUBLE value)
{
    DOUBLE_INT_UNION data;
    data.d_data = value;
    if (0x7ff00000 == data.i_data[1] && 0x00000000 == data.i_data[0])
        return 1;
    else if (0xfff00000 == data.i_data[1] && 0x00000000 == data.i_data[0])
        return -1;
    else
        return 0;
}

static BOOL IsBadData(DOUBLE data)
{
    if (IsNaN(data) || IsInf(data))
        return 1;
    else
        return 0;
}

static BOOL IsFatalError()
{
	if (IsBadData(g_pvt_result.pos_ecef_x) || IsBadData(g_pvt_result.pos_ecef_y) || IsBadData(g_pvt_result.pos_ecef_z))
		return 1;
	else
		return 0;
}

void SavePvtResult(const U8 *buffer, U32 msg_len)
{
    memcpy(&g_pvt_result, buffer, msg_len);
}

void TriggerResultPrinting()
{
    ks_os_flag_set_bits(flag_handle_result_printing, PRINT_RESULTS, SET_FLAG_MODE_OR);
}

S32 RtcmInput(S32 uart_id, char data)
{
    if (uart_id == -1)
    {
        return 0;
    }
    ks_rtk_rtcm_input((U8*)&data, 1);
    return 0;
}

void InitUartParser()
{
    for (S32 i = 0; i < 3; i++)
    {
        ks_driver_uart_register_protocol(i, RtcmInput);
    }
}

#define UART_ID_DEBUG 2

void RTK_DEBUG_OUTPUT(const U8* p_data, S32 len)
{
	ks_driver_uart_send_buffer(UART_ID_DEBUG, p_data, len);
}

void RTK_MONITOR_OUTPUT(const char* p_string)
{

}

void RtkPrintingTask(void *p_arg)
{
    ks_os_thread_vfp_enable();
    while (1)
    {
        ks_os_flag_wait(flag_handle_result_printing, PRINT_RESULTS, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
        ks_nmea_get_result(print_buffer,
        		(1 << NMEA_BIT_RMC) |
				(1 << NMEA_BIT_GGA) |
				(1 << NMEA_BIT_GLL) |
				(1 << NMEA_BIT_GSA) |
				(1 << NMEA_BIT_GSV) |
				(1 << NMEA_BIT_VTG) |
				(1 << NMEA_BIT_ZDA) |
				(1 << NMEA_BIT_GST) |
				(1 << NMEA_BIT_DHV)
				);
        ks_driver_uart_send_string(0, print_buffer);

        if (IsFatalError() == 0)
        {
        	ks_bb_feed_wdt();
        }
    }
}

void GetAndSetRtkConfig()
{
    RTK_CONFIG config;
    char buffer[128];
    ks_rtk_config_get(&config);
    sprintf(buffer, "Current RTK config: Max Age %dms\r\n", config.max_age_ms);
    ks_driver_uart_send_string(0, buffer);
    sprintf(buffer, "Set RTK Max Age to 120000ms\r\n");
    ks_driver_uart_send_string(0, buffer);
    config.max_age_ms = 120000;
    ks_rtk_config_set(&config);
}

/// RTK线程使用的优先级
#define RTK_THREAD_PRIORITY 20
void RtkInit()
{
    ks_os_flag_create(&flag_handle_result_printing, (char*)"flag_rtk_printing");
    ks_os_thread_create(&thread_handle_rtk_printing,
                        "rtk_printing_task",
                        RtkPrintingTask,
                        0,
                        19,
                        thread_stack_rtk_printing,
                        sizeof(thread_stack_rtk_printing),
                        0,
                        1
    );

    /// 初始化RTK中间件
    ks_rtk_init(RTK_THREAD_PRIORITY);
    /// 初始化串口输入用于RTCM输入
    InitUartParser();
    /// 获取和设置RTK配置
    GetAndSetRtkConfig();
    /// 允许RTK
    ks_rtk_enable(1);
    /// 开启debug信息
    ks_rtk_enable_debug(1);
    /// 不开启Monitor信息
    ks_rtk_enable_monitor(0);
    /// 开启BP调试信息输出
    ks_bb_set_debug_on(3);
}
