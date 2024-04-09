
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
#include "rtcm.h"
#include "rtcm_encoder.h"
#include "eph_type.h"
#include "pvt_meas_type.h"
#include "bb_config.h"
#include "pvt_type.h"
#include "ks_uart.h"
#include "ks_taskdef.h"
#include "msg_type.h"
#include "ks_exception.h"

#define _STACK_SIZE_MAIN        512
static S32 thread_stack_main[_STACK_SIZE_MAIN];
static OSHandle thread_handle_main;


#define SIGNAL_BIT(n)            (0x1 << ((n)-1))


/**
* 基带相关 BP -> AP 缓存到结构体数组中的三类数据:
* 1 观测量　
* 2 星历
* 3 pvt 结果
* 
*/

//1.观测量缓存数据
static U8 g_meas_data_buffer[16384] __attribute__((aligned(8)));
//2.星历缓存数据
static GpsEphemeris g_eph_gps[MAX_GPS_SAT_NUM] = {0};
static BdsEphemeris g_eph_bds[MAX_BDS_SAT_NUM] = {0};
static GlsEphemeris g_eph_gls[MAX_GLS_SAT_NUM] = {0};
static GalEphemeris g_eph_gal[MAX_GAL_SAT_NUM] = {0};
static GpsEphemeris g_eph_qzss[MAX_QZSS_SAT_NUM] = {0};
static GpsTgd g_tgd_gps[MAX_GPS_SAT_NUM] = {0};
static BdsTgd g_tgd_bds[MAX_BDS_SAT_NUM] = {0};
static GalTgd g_tgd_gal[MAX_GAL_SAT_NUM] = {0};
static GpsTgd g_tgd_qzss[MAX_QZSS_SAT_NUM] = {0};
static U64 g_eph_gps_update_flag = 0;
static U64 g_eph_bds_update_flag = 0;
static U64 g_eph_gls_update_flag = 0;
static U64 g_eph_gal_update_flag = 0;
static U64 g_eph_qzss_update_flag = 0;
//3.pvt缓存结果
static PvtResult g_pvt_result;

PvtMeas *MeasDataGet()
{
    return (PvtMeas *) g_meas_data_buffer;
}

GpsEphemeris *EphGpsGet(U8 prn)
{
    return (&g_eph_gps[prn - 1]);
}

BdsEphemeris *EphBdsGet(U8 prn)
{
    return (&g_eph_bds[prn - 1]);
}

GlsEphemeris *EphGlsGet(U8 prn)
{
    return (&g_eph_gls[prn - 1]);
}

GalEphemeris *EphGalGet(U8 prn)
{
    return (&g_eph_gal[prn - 1]);
}

GpsEphemeris *EphQzssGet(U8 prn)
{
    return (&g_eph_qzss[prn - 1]);
}

GpsTgd *TgdGpsGet(U8 prn)
{
    return (&g_tgd_gps[prn - 1]);
}

BdsTgd *TgdBdsGet(U8 prn)
{
    return (&g_tgd_bds[prn - 1]);
}

GalTgd *TgdGalGet(U8 prn)
{
    return (&g_tgd_gal[prn - 1]);
}

GpsTgd *TgdQzssGet(U8 prn)
{
    return (&g_tgd_qzss[prn - 1]);
}

PvtResult *PvtResultGet()
{
    return (&g_pvt_result);
}

U64 EphGpsGetUpdateFlag()
{
    return g_eph_gps_update_flag;
}

U64 EphBdsGetUpdateFlag()
{
    return g_eph_bds_update_flag;
}

U64 EphGlsGetUpdateFlag()
{
    return g_eph_gls_update_flag;
}

U64 EphGalGetUpdateFlag()
{
    return g_eph_gal_update_flag;
}

U64 EphQzssGetUpdateFlag()
{
    return g_eph_qzss_update_flag;
}

void EphAllClearUpdateFlag()
{
    g_eph_gps_update_flag = 0;
    g_eph_bds_update_flag = 0;
    g_eph_gls_update_flag = 0;
    g_eph_gal_update_flag = 0;
    g_eph_qzss_update_flag = 0;
}

static const U8 *ExtractData(const U8 *p_data_stream, void *p_extract_data, S32 data_bytes)
{
    memcpy(p_extract_data, p_data_stream, data_bytes);
    return p_data_stream + data_bytes;
}

void MeasDataProcess(BBMsgID msg_id, const U8 *buffer, U32 msg_len)
{
    memcpy(g_meas_data_buffer, buffer, msg_len);
    RtcmEncodeTrigger();
}

void NavEphProcess(BBMsgID msg_id, const U8 *buffer, U32 msg_len)
{
    const U8 *p_curr_data_stream = buffer;
    U32 bb_tag;
    U8 system_id, prn;
    U16 reserve;
    PTR ptr;
    p_curr_data_stream = ExtractData(p_curr_data_stream, &bb_tag, 4);
    p_curr_data_stream = ExtractData(p_curr_data_stream, &system_id, 1);
    p_curr_data_stream = ExtractData(p_curr_data_stream, &prn, 1);
    p_curr_data_stream = ExtractData(p_curr_data_stream, &reserve, 2);
    if (system_id == SYS_GPS)
    {
        memcpy(&g_eph_gps[prn - 1], p_curr_data_stream, sizeof(GpsEphemeris));
        g_eph_gps_update_flag |= 1ull << (prn - 1);
    }
    else if (system_id == SYS_BD3)
    {
        memcpy(&g_eph_bds[prn - 1], p_curr_data_stream, sizeof(BdsEphemeris));
        g_eph_bds_update_flag |= 1ull << (prn - 1);
    }
    else if (system_id == SYS_GLO)
    {
        memcpy(&g_eph_gls[prn - 1], p_curr_data_stream, sizeof(GlsEphemeris));
        g_eph_gls_update_flag |= 1ull << (prn - 1);
    }
    else if (system_id == SYS_GAL)
    {
        memcpy(&g_eph_gal[prn - 1], p_curr_data_stream, sizeof(GalEphemeris));
        g_eph_gal_update_flag |= 1ull << (prn - 1);
    }
    else if (system_id == SYS_QZS)
    {
        memcpy(&g_eph_qzss[prn - 1], p_curr_data_stream, sizeof(GpsEphemeris));
        g_eph_qzss_update_flag |= 1ull << (prn - 1);
    }
}

void NavTgdProcess(BBMsgID msg_id, const U8 *buffer, U32 msg_len)
{
    const U8 *p_curr_data_stream = buffer;
    U32 bb_tag;
    U8 system_id, prn;
    U16 reserve;
    PTR ptr;
    p_curr_data_stream = ExtractData(p_curr_data_stream, &bb_tag, 4);
    p_curr_data_stream = ExtractData(p_curr_data_stream, &system_id, 1);
    p_curr_data_stream = ExtractData(p_curr_data_stream, &prn, 1);
    p_curr_data_stream = ExtractData(p_curr_data_stream, &reserve, 2);
    if (system_id == SYS_GPS)
    {
        memcpy(&g_tgd_gps[prn - 1], p_curr_data_stream, sizeof(GpsTgd));
        g_eph_gps_update_flag |= 1ull << (prn - 1);
    }
    else if (system_id == SYS_BD3)
    {
        memcpy(&g_tgd_bds[prn - 1], p_curr_data_stream, sizeof(BdsTgd));
        g_eph_bds_update_flag |= 1ull << (prn - 1);
    }
    else if (system_id == SYS_GAL)
    {
        memcpy(&g_tgd_gal[prn - 1], p_curr_data_stream, sizeof(GalTgd));
        g_eph_gal_update_flag |= 1ull << (prn - 1);
    }
    else if (system_id == SYS_QZS)
    {
        memcpy(&g_tgd_qzss[prn - 1], p_curr_data_stream, sizeof(GpsTgd));
        g_eph_qzss_update_flag |= 1ull << (prn - 1);
    }
}

void PvtResultProcess(BBMsgID msg_id, const U8 *buffer, U32 msg_len)
{
    memcpy(&g_pvt_result, buffer, msg_len);
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
    ks_msg_register_callback(IDCavNavPvtMeas, MeasDataProcess);    /// 注册观测量消息处理函数
    ks_msg_register_callback(IDCavNavEph, NavEphProcess);          /// 注册星历消息处理函数
    ks_msg_register_callback(IDCavNavTgd, NavTgdProcess);          /// 注册群延迟消息处理函数
    ks_msg_register_callback(IDCavNavPvtResult, PvtResultProcess); /// 注册PVT结果消息处理函数
    /// 使能消息
    ks_msg_enable(IDCavNavPvtResult);
    ks_msg_enable(IDCavNavPvtMeas);
    ks_msg_enable(IDCavNavEph);
    ks_msg_enable(IDCavNavTgd);
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

void BaseBandInit()
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
    ks_bb_start(0);
}

void AppMainTask(void *p_arg)
{
    /// 允许浮点计算
    ks_os_thread_vfp_enable();
    /// 输出串口启动信息
    AppMainStartInfoPrint();
    /// 创建RTCM编码线程
    RtcmInit();
    /// 消息回调注册
    RegisterCallback();
    /// 获取SDK版本信息
    GetVersion();
    /// 基带初始化
    BaseBandInit();
    ks_driver_uart_send_string(0, "ks bb start \r\n");
}

/// SDK应用层初始化入口
void ks_app_init()
{
    ks_os_thread_create(&thread_handle_main,
                        "app_main_task",
                        AppMainTask,
                        0,
                        10,
                        thread_stack_main,
                        sizeof(thread_stack_main),
                        0,
                        1
    );
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
    ks_driver_uart_init(0, 460800,0);
    ks_driver_uart_init(1, 460800,0);
    ks_driver_uart_init(2, 460800,0);
    ks_driver_uart_init(3, 460800,0);
}



