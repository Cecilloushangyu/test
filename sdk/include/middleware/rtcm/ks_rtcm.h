#pragma once

#include "ks_bb.h"

#ifdef __cplusplus
extern "C" {
#endif

#include "ks_msg.h"

typedef enum
{
    RTCM_EPH_OUT_NONE,
    RTCM_EPH_OUT_UPDATED,
    RTCM_EPH_OUT_INTERVAL,
    RTCM_EPH_OUT_ALL
} RTCM_EPH_OUT_TYPE;

#define RTCM_MEAS_STATE_PSR_VALID  0x1      //!< 伪距有效
#define RTCM_MEAS_STATE_DOPPLER_VALID  0x2  //!< 多普勒有效
#define RTCM_MEAS_STATE_ADR_VALID  0x4      //!< 载波相位有效

typedef struct
{
    S32 msm_type;
    RTCM_EPH_OUT_TYPE eph_type;
    S32 eph_interval;   // 单位为秒
    S8 el_min;
    U8 meas_state_flag;
    S8 doppler_sign;
    U8 is_smoothed_psr;
    U8 is_compensate;
} RTCM_CONFIG;

void ks_rtcm_init(S32 thread_priority);

void ks_rtcm_config_get(RTCM_CONFIG *config_content);
BOOL ks_rtcm_config_set(RTCM_CONFIG *config_content);

void ks_rtcm_enable(BOOL b_enable);

/// @RTCM Streaming

typedef void (*RtcmCallback)(const U8 *buffer, U32 len);

void ks_rtcm_register_callback(RtcmCallback callback);
void ks_rtcm_deregister_callback(RtcmCallback callback);

BOOL ks_rtcm_enable_station_msg(S32 interval);
void ks_rtcm_set_base_station(U16 station_id, DOUBLE pos_ecef[3]);

/// @RTCM Polling

typedef enum
{
    RTCM1019 = 1019,  // GPS
    RTCM1020 = 1020,  // GLONASS
    RTCM1041 = 1041,  // IRNSS
    RTCM1042 = 1042,  // BDS
    RTCM1044 = 1044,  // QZSS
    RTCM1046 = 1046,  // Galileo I/NAV
} RTCMType;

typedef struct
{
    GNSSSystem system[8];
    U8 antenna_id;
    U8 msm_type;
    S8 el_min;
    U8 meas_state_flag;
    S8 doppler_sign;
    U8 is_smoothed_psr;
    U8 is_compensate;
} MSMConfig;
S32 ks_rtcm_get_msm(MSMConfig msm_config, U8 *rtcm_buff, S32 buff_len, S32 *rtcm_len, S32 *offset, S32 *len);
S32 ks_rtcm_get_eph(RTCMType rtcm_type, S32 *prn, U8 *rtcm_buff, S32 buff_len, S32 *rtcm_len);
S32 ks_rtcm_reset_eph_encode_state(RTCMType rtcm_type);

void ks_rtcm_get_base_station_info(U16 *p_station_id, DOUBLE pos_ecef[3]);
void ks_rtcm_set_receiver_and_antenna_desc(char *ant_des, char *ant_sn, U8 ant_setup_id, char *rcv_type_des);
void ks_rtcm_get_receiver_and_antenna_desc_info(char *ant_des, char *ant_sn, U8 *p_ant_setup_id, char *rcv_type_des);

int ks_rtcm_get_msm_legacy(U8 *rtcm_buff, int *rtcm_len, int msm_type);
int ks_rtcm_get_base_station(U8 *rtcm_buff, int *rtcm_len);
int ks_rtcm_get_receiver_and_antenna_desc(U8 *rtcm_buff, int *rtcm_len);

#ifdef __cplusplus
}
#endif
