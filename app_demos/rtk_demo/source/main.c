
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  main.h
 *
 * @brief SDK 应用层初始化入口，main主任务，创建app demo，启动基带等
 */

#include <string.h>
#include "ks_include.h"
#include "pvt_type.h"
#include "ks_uart.h"
#include "ks_taskdef.h"
#include "middleware/rtcm/ks_rtcm.h"
#include "middleware/nmea/ks_nmea.h"
#include "rtk/rtk_demo.h"
#include "ks_shell.h"
#include "ks_exception.h"

#define PROJECT_NAME "delos_rtk_demo"


#define _STACK_SIZE_MAIN        512
static S32 thread_stack_main[_STACK_SIZE_MAIN];
static OSHandle thread_handle_main;


#define SIGNAL_BIT(n)            (0x1 << ((n)-1))

void PvtResultProcess(BBMsgID msg_id, const U8 *buffer, U32 msg_len)
{
    SavePvtResult(buffer, msg_len);
    TriggerResultPrinting();
}

void AppMainStartInfoPrint()
{
    char t_string[128];
    sprintf(t_string, "App Main Start\r\n");
    ks_driver_uart_send_string(0, t_string);
}

void RegisterCallback()
{
    /// 注册接收BP消息的回调
    ks_msg_register_callback(IDCavNavPvtResult, PvtResultProcess); /// 注册PVT结果消息处理函数
    /// 使能消息
    ks_msg_enable(IDCavNavPvtResult);
}

void GetVersion()
{
    char print_buffer[128];
    BBVersionInfo version_info;
    ks_bb_get_version_info(&version_info);
    sprintf(print_buffer, "API Level %d, Versions SDK %s FW %s AP %s\r\n",
            version_info.sdk_protocol_level,
            version_info.sdk_version,
            version_info.fw_version,
            version_info.ap_version
    );
    ks_driver_uart_send_string(0, print_buffer);
}

#define UART_ID_RTCM 1
void RtcmOutput(const U8 *buffer, U32 len)
{
	ks_driver_uart_send_buffer(UART_ID_RTCM, buffer, len);
}

/// RTCM线程使用的优先级
#define RTCM_THREAD_PRIORITY 19
void RtcmInit()
{
    /// 初始化RTCM中间件
    ks_rtcm_init(RTCM_THREAD_PRIORITY);
    /// 注册RTCM回调函数
    ks_rtcm_register_callback(RtcmOutput);

    RTCM_CONFIG config;
    /// 获取RTCM编码配置
    ks_rtcm_config_get(&config);
    config.msm_type = 5;
    config.eph_type = RTCM_EPH_OUT_UPDATED;
    config.eph_interval = 10;
    config.el_min = -90;
    /// 配置RTCM编码
    ks_rtcm_config_set(&config);

    /// 允许RTCM
    ks_rtcm_enable(1);
}

void NmeaInit()
{
    NMEA_CONFIG config;
    /// 获取NMEA配置
    ks_nmea_config_get(&config);
    config.nmea_version = 4;
    config.threshold_enable = 1;
    config.threshold_pos_error_std_enu = 30;
    config.threshold_vel_error_std_enu = 3;
    /// 配置NMEA
    ks_nmea_config_set(&config);

    /// 允许NMEA
    ks_nmea_enable(1);
}

U32 BaseBandInit()
{
    /// 配置接收信号
    U32 signal_en = SIGNAL_BIT(ID_BD3B1I) | SIGNAL_BIT(ID_BD3B2I) | SIGNAL_BIT(ID_BD3B3I) |
    		SIGNAL_BIT(ID_BD3B1C) |  SIGNAL_BIT(ID_BD3B2a) |
			SIGNAL_BIT(ID_GPSL1CA) | SIGNAL_BIT(ID_GPSL2C) | SIGNAL_BIT(ID_GPSL5) |
			SIGNAL_BIT(ID_GALE1) | SIGNAL_BIT(ID_GALE5a) | SIGNAL_BIT(ID_GALE5b) |
			SIGNAL_BIT(ID_GLOG1) | SIGNAL_BIT(ID_GLOG2) |
			SIGNAL_BIT(ID_QZSL1CA) | SIGNAL_BIT(ID_QZSL2C) | SIGNAL_BIT(ID_QZSL5);
    ks_bb_set_signal(signal_en);
    /// 配置观测量输出频率
    ks_bb_set_nav_rate(1);
    /// 配置4路RX
    ks_bb_set_rx(RX_L1, RX_L2_L5, RX_B3_L6, RX_NULL);
    /// 启动基带
    return ks_bb_start(0);
}


void AppMainTask(void *p_arg)
{
    /// 允许浮点计算
    ks_os_thread_vfp_enable();
    /// 输出串口启动信息
    AppMainStartInfoPrint();
    /// 消息回调注册
    RegisterCallback();
    /// 获取SDK版本信息
    GetVersion();
    /// RTCM初始化
    RtcmInit();
    /// NMEA初始化
    NmeaInit();
    /// RTK初始化
    RtkInit();
    /// 基带初始化
    U32 ret = BaseBandInit();
    if (ret == KS_OK)
        ks_driver_uart_send_string(0, "ks bb start \r\n");
    else if (ret == KS_BB_START_ERROR_FW_VERSION_NOT_MATCH)
        ks_driver_uart_send_string(0,
        		"ks bb start failed: FW version not match,"
        		" make sure SDK files come from the same"
        		" SDK Release versoin\r\n");
    else
        ks_driver_uart_send_string(0, "ks bb start failed \r\n");
    /// 使能看门狗
    ks_bb_enable_wdt(1, 12);
}
#define _THREAD_PRI_APP_MAIN		24
/// SDK应用层初始化入口
void ks_app_init()
{
    ks_os_thread_create(&thread_handle_main,
                        "app_main_task",
                        AppMainTask,
                        0,
                        _THREAD_PRI_APP_MAIN,
                        thread_stack_main,
                        sizeof(thread_stack_main),
                        0,
                        1
    );
}

/// SDK驱动层初始化入口
void ks_drivers_init()
{
    ks_driver_uart_init(0, 460800, 0);
    ks_driver_uart_init(1, 460800, 0);
    ks_driver_uart_init(2, 460800, 0);
    ks_driver_uart_init(3, 460800, 0);
}

/// components 中间件初始化入口
void ks_components_init()
{
	/// 配置异常
	ks_exception_project_name_set(PROJECT_NAME);
	ks_exception_output_uart_set(0);
}


