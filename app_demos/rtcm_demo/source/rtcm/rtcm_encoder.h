#pragma once
#include "ks_datatypes.h"
#include "pvt_meas_type.h"

typedef struct SplitMask
{
    U64 cell_sat_mask[8];
    U32 cell_sig_mask[8];
    U8 ncell[8];
    U8 split_num;
} SplitMask;

typedef struct RtcmEncodeData
{
    U8 *buff;                //!< 编码数据缓存指针
    S32 msg_bit;            //!< RTCM信息比特计数
    S32 total_byte;        //!< 完整信息包（包含header和crc）字节计数，即待发送字节数
} RtcmEncodeData;

typedef struct RtcmStation
{
    DOUBLE antenna_ref_ecef_x;
    DOUBLE antenna_ref_ecef_y;
    DOUBLE antenna_ref_ecef_z;
    U16 reference_station_id;            //!< DF003
    U8 antenna_setup_id;                //!< DF031
    char antenna_descriptor[32];        //!< DF030
    char antenna_serial_number[32];    //!< DF033
    char receiver_type_descriptor[32];    //!< DF228
    char receiver_firmware_version[32]; //!< DF230
    char receiver_serial_number[32];    //!< DF232
} RtcmStation;

void EncodeRtcmEph(const void *p_eph, const void *p_tgd, U8 *p_rtcm_buff, S32 eph_type, S32 *rtcm_len_out);
void EncodeRtcmMsm(const PvtMeas *p_pvt_meas, U8 *p_rtcm_buff, S32 msm_type, U8 system_mask, S32 *rtcm_len_out);

