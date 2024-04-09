
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  rtcm.c
 *
 * @brief  rtcm 格式化输出任务实现　
 */


#include "ks_include.h"
#include "rtcm.h"
#include "rtcm_encoder.h"
#include "eph_type.h"
#include "bb_output.h"
#include "ks_uart.h"


#define  STACK_SIZE_RTCM_TASK          512
static S32 thread_stack_rtcm_task[STACK_SIZE_RTCM_TASK];
static OSHandle flag_handle_rtcm_task;
static OSHandle thread_handle_rtcm_task;

static U8 g_rtcm_output_buffer[1024 * 10] __attribute__((aligned(8)));

void RtcmEphOutput(U8 *rtcm_buff, U8 system, U8 prn, S32 *rtcm_len_out)
{
    void *p_eph;
    void *p_tgd;

    if (system == SYS_BD3)
    {
        p_eph = (void *) EphBdsGet(prn);
        p_tgd = (void *) TgdBdsGet(prn);
        EncodeRtcmEph(p_eph, p_tgd, rtcm_buff, 1042, rtcm_len_out);
    }
    else if (system == SYS_GPS)
    {
        p_eph = (void *) EphGpsGet(prn);
        p_tgd = (void *) TgdGpsGet(prn);
        EncodeRtcmEph(p_eph, p_tgd, rtcm_buff, 1019, rtcm_len_out);
    }
    else if (system == SYS_GLO)
    {
        p_eph = (void *) EphGlsGet(prn);
        p_tgd = NULL;
        EncodeRtcmEph(p_eph, p_tgd, rtcm_buff, 1020, rtcm_len_out);
    }
    else if (system == SYS_GAL)
    {
        p_eph = (void *) EphGalGet(prn);
        p_tgd = (void *) TgdGalGet(prn);
        EncodeRtcmEph(p_eph, p_tgd, rtcm_buff, 1045, rtcm_len_out);
        EncodeRtcmEph(p_eph, p_tgd, rtcm_buff, 1046, rtcm_len_out);
    }
    else if (system == SYS_QZS)
    {
        p_eph = (void *) EphQzssGet(prn);
        p_tgd = (void *) TgdQzssGet(prn);
        EncodeRtcmEph(p_eph, p_tgd, rtcm_buff, 1044, rtcm_len_out);
    }
}

void RtcmMsmOutput(U8 *rtcm_buff, S32 msm_type, U8 system_mask, S32 *rtcm_len_out)
{
    EncodeRtcmMsm(MeasDataGet(), rtcm_buff, msm_type, system_mask, rtcm_len_out);
}

void RtcmEncodeOutput()
{
    S32 len;

    // 观测量rtcm格式化输出
    RtcmMsmOutput(g_rtcm_output_buffer, 5, 0xFF, &len);
    ks_driver_uart_send_buffer(0, g_rtcm_output_buffer, len);

    // 星历rtcm格式化输出
    U64 eph_update_flag = 0;
    eph_update_flag = EphGpsGetUpdateFlag();
    if (eph_update_flag != 0)
    {
        for (S32 i = 0; i < 32; i++)
        {
            if (eph_update_flag & (1ull << i))
            {
                RtcmEphOutput(g_rtcm_output_buffer, SYS_GPS, i + 1, &len);
                ks_driver_uart_send_buffer(0, g_rtcm_output_buffer, len);
            }
        }
    }

    eph_update_flag = EphBdsGetUpdateFlag();
    if (eph_update_flag != 0)
    {
        for (S32 i = 0; i < 63; i++)
        {
            if (eph_update_flag & (1ull << i))
            {
                RtcmEphOutput(g_rtcm_output_buffer, SYS_BD3, i + 1, &len);
                ks_driver_uart_send_buffer(0, g_rtcm_output_buffer, len);
            }
        }
    }

    eph_update_flag = EphGlsGetUpdateFlag();
    if (eph_update_flag != 0)
    {
        for (S32 i = 0; i < 24; i++)
        {
            if (eph_update_flag & (1ull << i))
            {
                RtcmEphOutput(g_rtcm_output_buffer, SYS_GLO, i + 1, &len);
                ks_driver_uart_send_buffer(0, g_rtcm_output_buffer, len);
            }
        }
    }

    eph_update_flag = EphGalGetUpdateFlag();
    if (eph_update_flag != 0)
    {
        for (S32 i = 0; i < 40; i++)
        {
            if (eph_update_flag & (1ull << i))
            {
                RtcmEphOutput(g_rtcm_output_buffer, SYS_GAL, i + 1, &len);
                ks_driver_uart_send_buffer(0, g_rtcm_output_buffer, len);
            }
        }
    }

    eph_update_flag = EphQzssGetUpdateFlag();
    if (eph_update_flag != 0)
    {
        for (S32 i = 0; i < 10; i++)
        {
            if (eph_update_flag & (1ull << i))
            {
                RtcmEphOutput(g_rtcm_output_buffer, SYS_QZS, i + 1, &len);
                ks_driver_uart_send_buffer(0, g_rtcm_output_buffer, len);
            }
        }
    }

    EphAllClearUpdateFlag();
}

//rtcm  格式化输出任务　
void RtcmTask(void *p_arg)
{
    ks_os_thread_vfp_enable();
    while (1)
    {
        U32 flag = ks_os_flag_wait(flag_handle_rtcm_task, RTCM_ENCODE, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
        if (flag & RTCM_ENCODE)
        {
            RtcmEncodeOutput();
        }
    }
}

void RtcmEncodeTrigger()
{
    ks_os_flag_set_bits(flag_handle_rtcm_task, RTCM_ENCODE, SET_FLAG_MODE_OR);
}

void RtcmInit(void)
{
    ks_os_flag_create(&flag_handle_rtcm_task, "rtcm_task");
    ks_os_thread_create(&thread_handle_rtcm_task,
                        "rtcm_task",
                        RtcmTask,
                        0,
                        15,
                        thread_stack_rtcm_task,
                        sizeof(thread_stack_rtcm_task),
                        0,
                        1
    );
    ks_bb_set_debug_on(3);
}

