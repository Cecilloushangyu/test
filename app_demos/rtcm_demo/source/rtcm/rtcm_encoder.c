/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  rtcm.c
 *
 * @brief  rtcm 格式化输出api接口实现　
 */
#include <math.h>
#include <string.h>
#include "rtcm_encoder.h"
#include "pvt_meas_type.h"
#include "pvt_const.h"
#include "eph_type.h"
#include "bb_config.h"
#ifdef __PC__
#include "constants.h"
#else
#include "ks_include.h"
#endif


const S32 g_smooth = 1;
const S32 g_compensate = 1;

static RtcmEncodeData g_rtcm_encode_data;                //!< 缓存rtcm数据
static RtcmStation g_rtcm_station;            //!< 接收机信息
static DOUBLE g_rough_range[64];            //!< 存储每颗卫星的粗略伪距（各信号首先进行滑动平均）
static DOUBLE g_rough_inv_doppler_speed[64];    //!< 存储每颗卫星的粗略多普勒速度的相反数（与RTCM标准一致）


S32 rtcm3_preamble = 0xD3;        //!< 8bit
S32 rtcm3_reserved = 0;           //!< 6bit
S32 rtcm3_header_length = 24;     //!< preamble(8) + reserved(6) + message_length(10)



#define  RANGE_MS (LIGHTSPEED / 1000.)
#define  RANGE_MS_2P8 (LIGHTSPEED / 1000. / 256.)
#define  RANGE_MS_2P10 ( LIGHTSPEED / 1000. / 1024.)
#define  RANGE_MS_2P24 ( LIGHTSPEED / 1000. / 1024. / 1024. / 16)
#define  RANGE_MS_2P29 ( LIGHTSPEED / 1000. / 1024. / 1024. / 512)
#define  RANGE_MS_2P31 ( LIGHTSPEED / 1000. / 1024. / 1024. / 512 / 4)


DOUBLE P2_5 = 0.03125;
DOUBLE P2_6 = 0.015625;
DOUBLE P2_11 = 0.00048828125;
DOUBLE P2_19 = 1.907348632812500E-06;
DOUBLE P2_20 = 9.5367431640625E-7;
DOUBLE P2_29 = 1.862645149230957E-09;
DOUBLE P2_30 = 9.31322574615478515625E-10;
DOUBLE P2_31 = 4.656612873077393E-10;
DOUBLE P2_32 = 2.328306436538696e-10;
DOUBLE P2_33 = 1.164153218269348E-10;
DOUBLE P2_34 = 5.820766091346741e-11;
DOUBLE P2_40 = 9.094947017729282379150390625e-13;
DOUBLE P2_43 = 1.136868377216160E-13;
DOUBLE P2_46 = 1.421085471520200e-14;
DOUBLE P2_50 = 8.881784197001252E-16;
DOUBLE P2_66 = 1.355252715606881E-20;
DOUBLE P2_55 = 2.775557561562891E-17;
DOUBLE P2_59 = 1.734723475976807e-18;

/// <summary>
/// CRC-24Q计算用表，多项式为0,1,3,4,5,6,7,10,11,14,17,18,23,24
/// </summary>
const U32 table_crc24q[] = {
    0x000000, 0x864CFB, 0x8AD50D, 0x0C99F6, 0x93E6E1, 0x15AA1A, 0x1933EC, 0x9F7F17,
    0xA18139, 0x27CDC2, 0x2B5434, 0xAD18CF, 0x3267D8, 0xB42B23, 0xB8B2D5, 0x3EFE2E,
    0xC54E89, 0x430272, 0x4F9B84, 0xC9D77F, 0x56A868, 0xD0E493, 0xDC7D65, 0x5A319E,
    0x64CFB0, 0xE2834B, 0xEE1ABD, 0x685646, 0xF72951, 0x7165AA, 0x7DFC5C, 0xFBB0A7,
    0x0CD1E9, 0x8A9D12, 0x8604E4, 0x00481F, 0x9F3708, 0x197BF3, 0x15E205, 0x93AEFE,
    0xAD50D0, 0x2B1C2B, 0x2785DD, 0xA1C926, 0x3EB631, 0xB8FACA, 0xB4633C, 0x322FC7,
    0xC99F60, 0x4FD39B, 0x434A6D, 0xC50696, 0x5A7981, 0xDC357A, 0xD0AC8C, 0x56E077,
    0x681E59, 0xEE52A2, 0xE2CB54, 0x6487AF, 0xFBF8B8, 0x7DB443, 0x712DB5, 0xF7614E,
    0x19A3D2, 0x9FEF29, 0x9376DF, 0x153A24, 0x8A4533, 0x0C09C8, 0x00903E, 0x86DCC5,
    0xB822EB, 0x3E6E10, 0x32F7E6, 0xB4BB1D, 0x2BC40A, 0xAD88F1, 0xA11107, 0x275DFC,
    0xDCED5B, 0x5AA1A0, 0x563856, 0xD074AD, 0x4F0BBA, 0xC94741, 0xC5DEB7, 0x43924C,
    0x7D6C62, 0xFB2099, 0xF7B96F, 0x71F594, 0xEE8A83, 0x68C678, 0x645F8E, 0xE21375,
    0x15723B, 0x933EC0, 0x9FA736, 0x19EBCD, 0x8694DA, 0x00D821, 0x0C41D7, 0x8A0D2C,
    0xB4F302, 0x32BFF9, 0x3E260F, 0xB86AF4, 0x2715E3, 0xA15918, 0xADC0EE, 0x2B8C15,
    0xD03CB2, 0x567049, 0x5AE9BF, 0xDCA544, 0x43DA53, 0xC596A8, 0xC90F5E, 0x4F43A5,
    0x71BD8B, 0xF7F170, 0xFB6886, 0x7D247D, 0xE25B6A, 0x641791, 0x688E67, 0xEEC29C,
    0x3347A4, 0xB50B5F, 0xB992A9, 0x3FDE52, 0xA0A145, 0x26EDBE, 0x2A7448, 0xAC38B3,
    0x92C69D, 0x148A66, 0x181390, 0x9E5F6B, 0x01207C, 0x876C87, 0x8BF571, 0x0DB98A,
    0xF6092D, 0x7045D6, 0x7CDC20, 0xFA90DB, 0x65EFCC, 0xE3A337, 0xEF3AC1, 0x69763A,
    0x578814, 0xD1C4EF, 0xDD5D19, 0x5B11E2, 0xC46EF5, 0x42220E, 0x4EBBF8, 0xC8F703,
    0x3F964D, 0xB9DAB6, 0xB54340, 0x330FBB, 0xAC70AC, 0x2A3C57, 0x26A5A1, 0xA0E95A,
    0x9E1774, 0x185B8F, 0x14C279, 0x928E82, 0x0DF195, 0x8BBD6E, 0x872498, 0x016863,
    0xFAD8C4, 0x7C943F, 0x700DC9, 0xF64132, 0x693E25, 0xEF72DE, 0xE3EB28, 0x65A7D3,
    0x5B59FD, 0xDD1506, 0xD18CF0, 0x57C00B, 0xC8BF1C, 0x4EF3E7, 0x426A11, 0xC426EA,
    0x2AE476, 0xACA88D, 0xA0317B, 0x267D80, 0xB90297, 0x3F4E6C, 0x33D79A, 0xB59B61,
    0x8B654F, 0x0D29B4, 0x01B042, 0x87FCB9, 0x1883AE, 0x9ECF55, 0x9256A3, 0x141A58,
    0xEFAAFF, 0x69E604, 0x657FF2, 0xE33309, 0x7C4C1E, 0xFA00E5, 0xF69913, 0x70D5E8,
    0x4E2BC6, 0xC8673D, 0xC4FECB, 0x42B230, 0xDDCD27, 0x5B81DC, 0x57182A, 0xD154D1,
    0x26359F, 0xA07964, 0xACE092, 0x2AAC69, 0xB5D37E, 0x339F85, 0x3F0673, 0xB94A88,
    0x87B4A6, 0x01F85D, 0x0D61AB, 0x8B2D50, 0x145247, 0x921EBC, 0x9E874A, 0x18CBB1,
    0xE37B16, 0x6537ED, 0x69AE1B, 0xEFE2E0, 0x709DF7, 0xF6D10C, 0xFA48FA, 0x7C0401,
    0x42FA2F, 0xC4B6D4, 0xC82F22, 0x4E63D9, 0xD11CCE, 0x575035, 0x5BC9C3, 0xDD8538};

U32 CalcCrc24q(const U8 *src, S32 number_of_byte)
{
    S32 i;
    U32 crc = 0;
    for (i = 0; i < number_of_byte; i++)
        crc = (crc << 8) ^ table_crc24q[src[i] ^ (U8) (crc >> 16)];
    return (crc & 0xFFFFFF);
}

U32 GenSignalMask(S32 signal_id)
{
    return 1u << (signal_id - 1);
}

void SetBits(U8 *p_stream, S32 index, S32 len, U32 data)
{
    if ((len < 0) || (len > 32))
        return;
    U32 mask = 1 << (len - 1);
    int idx = (index / 8);

    for (S32 i = index; i < index + len; i++, mask >>= 1)
    {
        if (data & mask)
            p_stream[i / 8] |= 1 << (7 - (i % 8));
        else
            p_stream[i / 8] &= ~(1 << (7 - (i % 8)));
    }
}

static inline void SetBitsSignMag(U8 *p_stream, S32 index, S32 len, S32 data)
{
    SetBits(p_stream, index, 1, data < 0 ? 1 : 0);
    SetBits(p_stream, index + 1, len - 1, data < 0 ? -data : data);
}

static inline S32 GetSatelliteDataLenForMsmType(S32 msm_type)
{
	switch (msm_type)
	{
	case 1:	case 2:	case 3:
		return 10;
	case 4:	case 6:
		return 18;
	case 5:	case 7:
		return 36;
	default:
		break;
	}
	return 0;
}

static inline S32 GetSignalDataLenForMsmType(S32 msm_type)
{
	switch (msm_type)
	{
	case 1: return 15;
	case 2: return 27;
	case 3: return 42;
	case 4: return 48;
	case 5: return 63;
	case 6: return 65;
	case 7: return 80;
	default:
		break;
	}
	return 0;
}

S32 GetMaxSupportRtcmSignal(U8 rtcm_system)
{
    if (rtcm_system == SYS_GPS)
        return 3;
    if (rtcm_system == SYS_BD3)
        return 10;
    if (rtcm_system == SYS_GLO)
        return 2;
    if (rtcm_system == SYS_GAL)
        return 3;
    if (rtcm_system == SYS_QZS)
        return 3;
    return 0;
}

BOOL CovertToRtcmSignal(U8 signal, U8 target_system, U32 *rtcm_signal_mask_out, S32 *store_index_out)
{
    //!< store index的顺序必须与Mask的大小顺序一致!!!
    S32 store_index;
    store_index = -1;
    U8 curr_system = SYS_NULL;
    U32 rtcm_signal_mask = 0;
    switch (signal)
    {
       // GPS Support: 2-L1(C/A), 17-L2C(M+L), 23-L5(Q)
    case ID_GPSL1CA:    
        rtcm_signal_mask = GenSignalMask(2);
        store_index = 0;
        curr_system = SYS_GPS;
        break;
    case ID_GPSL2C:
        rtcm_signal_mask = GenSignalMask(17);
        store_index = 1;
        curr_system = SYS_GPS;
        break;
    case ID_GPSL5:
        rtcm_signal_mask = GenSignalMask(23);
        store_index = 2;
        curr_system = SYS_GPS;
        break;
        // QZSS Support: 2-L1(C/A), 17-L2C(M+L), 23-L5(Q)
    case ID_QZSL1CA:
        rtcm_signal_mask = GenSignalMask(2);
        store_index = 0;
        curr_system = SYS_QZS;
        break;
    case ID_QZSL2C:
        rtcm_signal_mask = GenSignalMask(17);
        store_index = 1;
        curr_system = SYS_QZS;
        break;
    case ID_QZSL5:
        rtcm_signal_mask = GenSignalMask(23);
        store_index = 2;
        curr_system = SYS_QZS;
        break;
        // BDS Support: 2-B1I, 6-B1A, 8-B3I, 9-B3Q, 11-B3A, 12-B3AE, 14-B2I, 23-B2a, 25-B2b, 31-B1C
    case ID_BD3B1I:
        rtcm_signal_mask = GenSignalMask(2);
        store_index = 0;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B1A:
        rtcm_signal_mask = GenSignalMask(6);
        store_index = 1;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B3I:
        rtcm_signal_mask = GenSignalMask(8);
        store_index = 2;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B3Q:
        rtcm_signal_mask = GenSignalMask(9);
        store_index = 3;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B3A:
        rtcm_signal_mask = GenSignalMask(11);
        store_index = 4;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B3AE:
        rtcm_signal_mask = GenSignalMask(12);
        store_index = 5;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B2I:
        rtcm_signal_mask = GenSignalMask(14);
        store_index = 6;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B2a:
        rtcm_signal_mask = GenSignalMask(23);
        store_index = 7;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B2b:
        rtcm_signal_mask = GenSignalMask(25);
        store_index = 8;
        curr_system = SYS_BD3;
        break;
    case ID_BD3B1C:
    	rtcm_signal_mask = GenSignalMask(31);
    	store_index = 9;
    	curr_system = SYS_BD3;
    	break;
        // GLS Support: 2-G1(C/A), 8-G2(C/A)
    case ID_GLOG1:
        rtcm_signal_mask = GenSignalMask(2);
        store_index = 0;
        curr_system = SYS_GLO;
        break;
    case ID_GLOG2:
        rtcm_signal_mask = GenSignalMask(8);
        store_index = 1;
        curr_system = SYS_GLO;
        break;
        // GAL Support: 2-E1(C), 23-E5a(Q), 15-E5b(Q)
    case ID_GALE1:
        rtcm_signal_mask = GenSignalMask(2);
        store_index = 0;
        curr_system = SYS_GAL;
        break;
    case ID_GALE5b:
        rtcm_signal_mask = GenSignalMask(15);
        store_index = 1;
        curr_system = SYS_GAL;
        break;
    case ID_GALE5a:
        rtcm_signal_mask = GenSignalMask(23);
        store_index = 2;
        curr_system = SYS_GAL;
        break;    
    default:
        break;
    }

    *store_index_out = store_index;
    *rtcm_signal_mask_out = rtcm_signal_mask;

    if ((store_index >= 0) && (curr_system == target_system))
        return TRUE;
    return FALSE;
}

S32 GetMsmNumber(S32 msm_type, U8 rtcm_system)
{
    if (rtcm_system == SYS_GPS)
        return 1070 + msm_type;
    if (rtcm_system == SYS_BD3)
        return 1120 + msm_type;
    if (rtcm_system == SYS_GLO)
        return 1080 + msm_type;
    if (rtcm_system == SYS_GAL)
        return 1090 + msm_type;
    if (rtcm_system == SYS_QZS)
        return 1110 + msm_type;
    return 0;
}

S32 GetGnssEpochTime(U8 rtcm_system, const PvtMeas *p_pvt_meas)
{
    if (rtcm_system == SYS_GPS || rtcm_system == SYS_QZS)
        return p_pvt_meas->gps_local_time_ms - p_pvt_meas->gps_time_adjust_ms * g_compensate;
    if (rtcm_system == SYS_BD3)
        return p_pvt_meas->bds_local_time_ms - p_pvt_meas->bds_time_adjust_ms * g_compensate;
    if (rtcm_system == SYS_GAL)
        return p_pvt_meas->gal_local_time_ms - p_pvt_meas->gal_time_adjust_ms * g_compensate;
    if (rtcm_system == SYS_GLO)
    {
        S32 epoch_time = p_pvt_meas->gls_local_time_ms - p_pvt_meas->gls_time_adjust_ms * g_compensate;
        epoch_time &= 0x7FFFFFF;
        S32 day_of_week = p_pvt_meas->gls_day_in_week & 0x7;
        if (day_of_week != 7)
        {
        	if ((epoch_time < 2000) && (p_pvt_meas->gls_rcv_time > 86398))
        	{
        		day_of_week++;
        		if (day_of_week == 7)
        			day_of_week = 0;
        	}
        	else if ((epoch_time > 86398000) && (p_pvt_meas->gls_rcv_time < 2))
        	{
        		day_of_week--;
        		if (day_of_week < 0)
        			day_of_week = 6;
        	}
        }
        epoch_time |= day_of_week << 27;
        return epoch_time;
    }
    return 0;
}

U32 MapStoreIndexToRtcmSignalIndex(U8 rtcm_system, S32 store_index)
{
    //!< 检查返回值应随store_index递增
    if (rtcm_system == SYS_GPS)
    {
        switch (store_index)
        {
        case 0: return 2;
        case 1: return 17;
        case 2: return 23;
        default:
            break;
        }
    }
    else if (rtcm_system == SYS_BD3)
    {
        switch (store_index)
        {
        case 0: return 2;
        case 1: return 6;
        case 2: return 8;
        case 3: return 9;
        case 4: return 11;
        case 5: return 12;
        case 6: return 14;
        case 7: return 23;
        case 8: return 25;
        case 9: return 31;
        default:
            break;
        }
    }
    else if (rtcm_system == SYS_GLO)
    {
        switch (store_index)
        {
        case 0: return 2;
        case 1: return 8;
        default:
            break;
        }
    }
    else if (rtcm_system == SYS_GAL)
    {
        switch (store_index)
        {
        case 0: return 2;
        case 1: return 15;
        case 2: return 23;
        default:
            break;
        }
    }
    else if (rtcm_system == SYS_QZS)
    {
        switch (store_index)
        {
        case 0: return 2;
        case 1: return 17;
        case 2: return 23;
        default:
            break;
        }
    }
    return 0;
}
//void SplitMsm(U8 rtcm_system, U64 cell_sat_mask, U32 cell_sig_mask, U8 cell_all_mask[64][10], SplitMask* split_mask_out)
void SplitMsm(U8 rtcm_system, U64 cell_sat_mask, U32 cell_sig_mask, U8(*cell_all_mask)[10], SplitMask *split_mask_out)
{
    SplitMask split_mask = {0};
    S32 sat_index = 0;
    while (1)
    {
        S32 nsat = 0, nsig = 0, ncell = 0;
        U32 curr_sig_mask = 0;
        U64 curr_sat_mask = 0;
        BOOL fail_flag = FALSE; //!< 添加失败标识
        for (S32 i = sat_index; i < 64; i++)
        {
            if ((1ull << i) & cell_sat_mask)
            {
                if (((nsat + 1) * (nsig)) > 64)
                {
                    sat_index = i;
                    fail_flag = TRUE;
                    break;
                }
                U32 back_up_sig_mask = curr_sig_mask;
                U64 back_up_sat_mask = curr_sat_mask;
                S32 back_up_ncell = ncell;
                for (S32 j = 0; j < GetMaxSupportRtcmSignal(rtcm_system); j++)
                {
                    U32 rtcm_sig_mask = GenSignalMask(MapStoreIndexToRtcmSignalIndex(rtcm_system, j));
                    if ((rtcm_sig_mask & cell_sig_mask) == 0)
                        continue;
                    if (cell_all_mask[i][j])
                    {
                        ncell++;
                        curr_sat_mask |= 1ull << i;
                        if ((curr_sig_mask & rtcm_sig_mask) == 0)
                        {
                            curr_sig_mask |= rtcm_sig_mask;
                            nsig++;
                        }
                    }
                }
                if (((nsat + 1) * nsig) > 64)
                {
                    sat_index = i;
                    curr_sig_mask = back_up_sig_mask;
                    curr_sat_mask = back_up_sat_mask;
                    ncell = back_up_ncell;
                    fail_flag = TRUE;
                    break;
                }
                nsat++;
            }
        }
        split_mask.cell_sat_mask[split_mask.split_num] = cell_sat_mask & curr_sat_mask;
        split_mask.cell_sig_mask[split_mask.split_num] = curr_sig_mask;
        split_mask.ncell[split_mask.split_num] = ncell;
        split_mask.split_num++;
        if (!fail_flag)
            break;
    }

    memcpy(split_mask_out, &split_mask, sizeof(SplitMask));

}

void GetRoughRangeBit(DOUBLE rough_range, U32 *rough_range_ms_out, U32 *rough_range_module_1ms_out)
{
    U32 rough_range_ms = 255;
    U32 rough_range_module_1ms = 255;

    if ((rough_range > 0) && (rough_range < 255 * RANGE_MS))
    {
        S32 temp = (S32) (rough_range / RANGE_MS_2P10 + 0.5);
        rough_range_ms = temp >> 10;
        rough_range_module_1ms = temp & 0x3FF;
    }

    *rough_range_ms_out = rough_range_ms;
    *rough_range_module_1ms_out = rough_range_module_1ms;
}

void EncodeSatelliteData(S32 msm_type, const PvtChanMeas *meas, S32 start_offset, S32 sat_index, S32 nsat)
{
    S32 index = meas->prn - 1;

    g_rough_range[index] = round(g_rough_range[index] / RANGE_MS_2P10) * RANGE_MS_2P10;
    g_rough_inv_doppler_speed[index] = round(g_rough_inv_doppler_speed[index]);

    U32 rough_range_ms, rough_range_module_1ms;
    GetRoughRangeBit(g_rough_range[index], &rough_range_ms, &rough_range_module_1ms);
    S32 rough_doppler_speed_int = (S32) (g_rough_inv_doppler_speed[index]);

    if (msm_type == 1 || msm_type == 2 || msm_type == 3)
    {
        SetBits(g_rtcm_encode_data.buff, start_offset + sat_index * 10, 10, rough_range_module_1ms);               //!< rough range module 1ms
    }
    else if (msm_type == 4 || msm_type == 6)
    {
        SetBits(g_rtcm_encode_data.buff, start_offset + sat_index * 8, 8, rough_range_ms);                         //!< rough range integer ms
        SetBits(g_rtcm_encode_data.buff, start_offset + 8 * nsat + sat_index * 10, 10, rough_range_module_1ms);    //!< rough range module 1ms
    }
    else if (msm_type == 5 || msm_type == 7)
    {
        SetBits(g_rtcm_encode_data.buff, start_offset + sat_index * 8, 8, rough_range_ms);                         //!< rough range integer ms
        S32 ext_info = 0;
        if (meas->signal == ID_GLOG1 || meas->signal == ID_GLOG2)
            ext_info = meas->slot_freq + 7;
        SetBits(g_rtcm_encode_data.buff, start_offset + 8 * nsat + sat_index * 4, 4, ext_info);                    //!< extend info
        SetBits(g_rtcm_encode_data.buff, start_offset + 12 * nsat + sat_index * 10, 10, rough_range_module_1ms);   //!< rough range module 1ms
        SetBits(g_rtcm_encode_data.buff, start_offset + 22 * nsat + sat_index * 14, 14, rough_doppler_speed_int);  //!< rough doppler
    }
}

S32 GetFineRangeBit(DOUBLE rough_range, DOUBLE psr)
{
    DOUBLE fine_range = psr - rough_range;
    S32 fine_range_int = 0x4000;
    if (fabs(fine_range) < RANGE_MS_2P10)
        fine_range_int = (S32) (round(fine_range / RANGE_MS_2P24));
    return fine_range_int;
}

S32 GetFinePhaseBit(DOUBLE rough_range, DOUBLE adr)
{
    DOUBLE fine_phase = adr - rough_range;
    S32 fine_phase_int = 0x200000;
    if (fabs(fine_phase) < RANGE_MS_2P8)
        fine_phase_int = (S32) (round(fine_phase / RANGE_MS_2P29));
    return fine_phase_int;
}

S32 GetFineRangeBitExt(DOUBLE rough_range, DOUBLE psr)
{
    DOUBLE fine_range = psr - rough_range;
    S32 fine_range_int = 0x80000;
    if (fabs(fine_range) < RANGE_MS_2P10)
        fine_range_int = (S32) (round(fine_range / RANGE_MS_2P29));
    return fine_range_int;
}

S32 GetFinePhaseBitExt(DOUBLE rough_range, DOUBLE adr)
{
    DOUBLE fine_phase = adr - rough_range;
    S32 fine_phase_int = 0x800000;
    if (fabs(fine_phase) < RANGE_MS_2P8)
        fine_phase_int = (S32) (round(fine_phase / RANGE_MS_2P31));
    return fine_phase_int;
}

S32 GetLockIndicator(S32 lock_time_ms)
{
    if (lock_time_ms < 32)
        return 0;
    if (lock_time_ms < 64)
        return 1;
    if (lock_time_ms < 128)
        return 2;
    if (lock_time_ms < 256)
        return 3;
    if (lock_time_ms < 512)
        return 4;
    if (lock_time_ms < 1024)
        return 5;
    if (lock_time_ms < 2048)
        return 6;
    if (lock_time_ms < 4096)
        return 7;
    if (lock_time_ms < 8192)
        return 8;
    if (lock_time_ms < 16384)
        return 9;
    if (lock_time_ms < 32768)
        return 10;
    if (lock_time_ms < 65536)
        return 11;
    if (lock_time_ms < 131072)
        return 12;
    if (lock_time_ms < 262144)
        return 13;
    if (lock_time_ms < 524288)
        return 14;
    return 15;
}

S32 GetLockIndicatorExt(S32 lock_time_ms)
{
	if (lock_time_ms < 64)
		return lock_time_ms;
	if (lock_time_ms < 128)
		return (lock_time_ms + 64)/ 2;
	if (lock_time_ms < 256)
		return (lock_time_ms + 256)/ 4;
	if (lock_time_ms < 512)
		return (lock_time_ms + 768)/ 8;
	if (lock_time_ms < 1024)
		return (lock_time_ms + 2048)/ 16;
	if (lock_time_ms < 2048)
		return (lock_time_ms + 5120)/ 32;
	if (lock_time_ms < 4096)
		return (lock_time_ms + 12288)/ 64;
	if (lock_time_ms < 8192)
		return (lock_time_ms + 28672)/ 128;
	if (lock_time_ms < 16384)
		return (lock_time_ms + 65536)/ 256;
	if (lock_time_ms < 32768)
		return (lock_time_ms + 147456)/ 512;
	if (lock_time_ms < 65536)
		return (lock_time_ms + 327680)/ 1024;
	if (lock_time_ms < 131072)
		return (lock_time_ms + 720896)/ 2048;
	if (lock_time_ms < 262144)
		return (lock_time_ms + 1572864)/ 4096;
	if (lock_time_ms < 524288)
		return (lock_time_ms + 3407872)/ 8192;
	if (lock_time_ms < 1048576)
		return (lock_time_ms + 7340032)/ 16384;
	if (lock_time_ms < 2097152)
		return (lock_time_ms + 15728640)/ 32768;
	if (lock_time_ms < 4194304)
		return (lock_time_ms + 33554432)/ 65536;
	if (lock_time_ms < 8388608)
		return (lock_time_ms + 71303168)/ 131072;
	if (lock_time_ms < 16777216)
		return (lock_time_ms + 150994944)/ 262144;
	if (lock_time_ms < 33554432)
		return (lock_time_ms + 318767104)/ 524288;
	if (lock_time_ms < 67108864)
		return (lock_time_ms + 671088640)/ 1048576;
	return (lock_time_ms + 1409286144)/ 2097152;
}

S32 GetFineDopplerSpeedBit(DOUBLE rough_doppler_speed, DOUBLE doppler_speed)
{
    DOUBLE fine_doppler_speed = doppler_speed - rough_doppler_speed;
    S32 fine_doppler_speed_int = 0x4000;
    if (fabs(fine_doppler_speed) < 1.6383)
        fine_doppler_speed_int = (S32) (round(fine_doppler_speed / 0.0001));
    return fine_doppler_speed_int;
}

void EncodeSignalData(S32 msm_type,
                      const PvtChanMeas *meas,
                      DOUBLE clk_drift,
                      S32 start_offset,
                      S32 cell_index,
                      S32 ncell)
{
    S32 index = meas->prn - 1;
    S32 fine_range_int = 0, fine_phase_int = 0, cnr_int = 0;
    S32 lock_indicator = 0;
    S32 half_cycle_ambiguity = 0;   //!< pvt只在极性确定下才标记载波相位有效，所以不存在半周模糊度

    if (msm_type == 1)
    {
    	fine_range_int = GetFineRangeBit(g_rough_range[index],(g_smooth ? meas->psr_smooth : meas->psr) + meas->compensate_meter * g_compensate);
    }
    else if (msm_type == 2)
    {
    	fine_phase_int = GetFinePhaseBit(g_rough_range[index], meas->adr + meas->compensate_meter * g_compensate);
    	lock_indicator = GetLockIndicator(meas->lock_time_ms);
    }
    else if (msm_type == 3)
    {
    	fine_range_int = GetFineRangeBit(g_rough_range[index],(g_smooth ? meas->psr_smooth : meas->psr) + meas->compensate_meter * g_compensate);
    	fine_phase_int = GetFinePhaseBit(g_rough_range[index], meas->adr + meas->compensate_meter * g_compensate);
    	lock_indicator = GetLockIndicator(meas->lock_time_ms);
    }
    else if (msm_type == 4 || msm_type == 5)
    {
    	fine_range_int = GetFineRangeBit(g_rough_range[index],(g_smooth ? meas->psr_smooth : meas->psr) + meas->compensate_meter * g_compensate);
    	fine_phase_int = GetFinePhaseBit(g_rough_range[index], meas->adr + meas->compensate_meter * g_compensate);
    	cnr_int = (meas->cn0 + 50) / 100;
    	lock_indicator = GetLockIndicator(meas->lock_time_ms);
    }
    else if (msm_type == 6 || msm_type == 7)
    {
    	fine_range_int = GetFineRangeBitExt(g_rough_range[index],(g_smooth ? meas->psr_smooth : meas->psr) + meas->compensate_meter * g_compensate);
    	fine_phase_int = GetFinePhaseBitExt(g_rough_range[index], meas->adr + meas->compensate_meter * g_compensate);
    	cnr_int = (S32)(meas->cn0 / 100. * 16 + 0.5);
    	lock_indicator = GetLockIndicatorExt(meas->lock_time_ms);
    }

    S32 doppler_speed_int = GetFineDopplerSpeedBit(g_rough_inv_doppler_speed[index], -(meas->doppler_speed - clk_drift * g_compensate));

    if (msm_type == 1)
    {
    	SetBits(g_rtcm_encode_data.buff, start_offset + cell_index * 15, 15, fine_range_int);                      //!< DF400, fine range
    }
    else if (msm_type == 2)
    {
    	SetBits(g_rtcm_encode_data.buff, start_offset + cell_index * 22, 22, fine_phase_int);         //!< DF401, fine phase
    	SetBits(g_rtcm_encode_data.buff, start_offset + 22 * ncell + cell_index * 4, 4, lock_indicator);           //!< DF402, lock
    	SetBits(g_rtcm_encode_data.buff, start_offset + 26 * ncell + cell_index * 1, 1, half_cycle_ambiguity);     //!< DF420, half-cycle
    }
    else if (msm_type == 3)
    {
    	SetBits(g_rtcm_encode_data.buff, start_offset + cell_index * 15, 15, fine_range_int);                      //!< DF400, fine range
    	SetBits(g_rtcm_encode_data.buff, start_offset + 15 * ncell + cell_index * 22, 22, fine_phase_int);         //!< DF401, fine phase
    	SetBits(g_rtcm_encode_data.buff, start_offset + 37 * ncell + cell_index * 4, 4, lock_indicator);           //!< DF402, lock
    	SetBits(g_rtcm_encode_data.buff, start_offset + 41 * ncell + cell_index * 1, 1, half_cycle_ambiguity);     //!< DF420, half-cycle
    }
    else if (msm_type == 4 || msm_type == 5)
    {
        SetBits(g_rtcm_encode_data.buff, start_offset + cell_index * 15, 15, fine_range_int);                      //!< DF400, fine range
        SetBits(g_rtcm_encode_data.buff, start_offset + 15 * ncell + cell_index * 22, 22, fine_phase_int);         //!< DF401, fine phase
        SetBits(g_rtcm_encode_data.buff, start_offset + 37 * ncell + cell_index * 4, 4, lock_indicator);           //!< DF402, lock
        SetBits(g_rtcm_encode_data.buff, start_offset + 41 * ncell + cell_index * 1, 1, half_cycle_ambiguity);     //!< DF420, half-cycle
        SetBits(g_rtcm_encode_data.buff, start_offset + 42 * ncell + cell_index * 6, 6, cnr_int);                  //!< DF403, cnr
        if (msm_type == 5)                    
            SetBits(g_rtcm_encode_data.buff, start_offset + 48 * ncell + cell_index * 15, 15, doppler_speed_int);  //!< DF404, doppler        
    }
    else if (msm_type == 6 || msm_type == 7)
    {
    	SetBits(g_rtcm_encode_data.buff, start_offset + cell_index * 20, 20, fine_range_int);                      //!< DF400, fine range
    	SetBits(g_rtcm_encode_data.buff, start_offset + 20 * ncell + cell_index * 24, 24, fine_phase_int);         //!< DF401, fine phase
    	SetBits(g_rtcm_encode_data.buff, start_offset + 44 * ncell + cell_index * 10, 10, lock_indicator);         //!< DF402, lock
    	SetBits(g_rtcm_encode_data.buff, start_offset + 54 * ncell + cell_index * 1, 1, half_cycle_ambiguity);     //!< DF420, half-cycle
    	SetBits(g_rtcm_encode_data.buff, start_offset + 55 * ncell + cell_index * 10, 10, cnr_int);                //!< DF403, cnr
    	if (msm_type == 7)
    		SetBits(g_rtcm_encode_data.buff, start_offset + 65 * ncell + cell_index * 15, 15, doppler_speed_int);  //!< DF404, doppler
    }
}

void ComposeRtcmMsg()
{
    while (g_rtcm_encode_data.msg_bit % 8)
    {
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + g_rtcm_encode_data.msg_bit, 1, 0);
        g_rtcm_encode_data.msg_bit++;
    }
    S32 msg_byte = g_rtcm_encode_data.msg_bit / 8;
    if (msg_byte > 1023)
    {
        g_rtcm_encode_data.total_byte = 0;
        return;
    }
    SetBits(g_rtcm_encode_data.buff, 0, 8, rtcm3_preamble);
    SetBits(g_rtcm_encode_data.buff, 8, 6, rtcm3_reserved);
    SetBits(g_rtcm_encode_data.buff, 14, 10, msg_byte);
    U32 crc = CalcCrc24q(g_rtcm_encode_data.buff, msg_byte + 3);
    SetBits(g_rtcm_encode_data.buff, (msg_byte + 3) * 8, 24, crc);
    g_rtcm_encode_data.total_byte = msg_byte + 6;
}

int BitCount32(U32 n)
{
    unsigned int c = 0; // 计数器
    while (n > 0)
    {
        if ((n & 1) == 1) // 当前位是1
            ++c; // 计数器加1
        //n >>=1 ; // 移位
        n = n >> 1;
    }
    return c;
}

int BitCount64(U64 n)
{
    unsigned int c = 0; // 计数器
    while (n > 0)
    {
        if ((n & 1ull) == 1) // 当前位是1
            ++c; // 计数器加1
        //n >>=1 ; // 移位
        n = n >> 1;
    }
    return c;
}

U32 getU64IndexBit(U64 n, U32 idx)
{
    U64 temp = 1;
    temp = temp << idx;
    temp = n & temp;
    return temp > 0 ? 1 : 0;
}

U32 getU32IndexBit(U32 n, U32 idx)
{
    U32 temp = 1;
    temp = temp << idx;
    temp = n & temp;
    return temp > 0 ? 1 : 0;

}

S32 IsMeaValidState(U16 state)
{
//    return (state & (PVT_CHAN_MEAS_STATE_SET_RTCM_VALID | PVT_CHAN_MEAS_STATE_EL_LOW))
//        == PVT_CHAN_MEAS_STATE_SET_RTCM_VALID;
    return (state & (PVT_CHAN_MEAS_STATE_SET_RTCM_VALID))
        == PVT_CHAN_MEAS_STATE_SET_RTCM_VALID;
}

GNSSSystem GetSystemFromSignal(U8 signal)
{
    switch ((GNSSSignalID)signal) {
        case ID_GPSL1C:
        case ID_GPSL1CA:
        case ID_GPSL2C:
        case ID_GPSL5:
            return SYS_GPS;
        case ID_GLOG1:
        case ID_GLOG2:
            return SYS_GLO;
        case ID_GALE1:
        case ID_GALE5a:
        case ID_GALE5b:
        case ID_GALE6:
            return SYS_GAL;
        case ID_BD3B1I:
        case ID_BD3B1C:
        case ID_BD3B1A:
        case ID_BD3B2a:
        case ID_BD3B2b:
        case ID_BD3B3I:
        case ID_BD3B3Q:
        case ID_BD3B3A:
        case ID_BD3B3AE:
        case ID_BD3B2I:
            return SYS_BD3;
        case ID_QZSL1CA:
        case ID_QZSL2C:
        case ID_QZSL5:
            return SYS_QZS;
        default:
            return SYS_NULL;
    }
}

U32 GetValidSystemFlags(const PvtMeas *p_pvt_meas)
{
    U32 result = 0;     // 每一位标记系统是否有需要编码的星
    const PvtChanMeas *p_chan_meas_base = (PvtChanMeas *) (&p_pvt_meas->meas_pointer);
    for (S32 i = 0; i < p_pvt_meas->meas_count; i++)
    {
        const PvtChanMeas *p_chan_meas = &p_chan_meas_base[i];
        if (IsMeaValidState(p_chan_meas->state))
        {
            GNSSSystem system_id = GetSystemFromSignal(p_chan_meas->signal);
            result |= (1 << system_id);
        }
    }
    return result;
}


S32 EncodeMsm(S32 msm_type, U8 rtcm_system, const PvtMeas *p_pvt_meas, BOOL b_multiple)
{

    const PvtChanMeas *p_chan_meas_base = (PvtChanMeas *) (&p_pvt_meas->meas_pointer);
    U32 rtcm_signal_mask;
    U8 cell_all_mask[64][10];   //!< 最大支持64颗星，每颗星10个信号
    U64 cell_sat_mask = 0;      //!< 与填入的bit反序
    U32 cell_sig_mask = 0;      //!< 与填入的bit反序
    S32 nsat = 0, nsig = 0, ncell = 0;
    S8 psr_cnt[64];
    memset(cell_all_mask, 0, sizeof(cell_all_mask));

    for (S32 i = 0; i < p_pvt_meas->meas_count; i++)
    {
        const PvtChanMeas *p_chan_meas = &p_chan_meas_base[i];
        S32 ms_count = -1, store_index;
        DOUBLE wave_length = 0.;

        if (IsMeaValidState(p_chan_meas->state))
        {
            if (CovertToRtcmSignal(p_chan_meas->signal, rtcm_system, &rtcm_signal_mask, &store_index))
            {
                S32 index = p_chan_meas->prn - 1;
                if (((1ull << index) & cell_sat_mask) == 0)
                {
                    cell_sat_mask |= 1ull << index;
                    nsat++;
                    g_rough_range[index] = (g_smooth ? p_chan_meas->psr_smooth : p_chan_meas->psr)
                        + p_chan_meas->compensate_meter * g_compensate;
                    g_rough_inv_doppler_speed[index] = -(p_chan_meas->doppler_speed
                        - p_pvt_meas->clk_drift * g_compensate);   // 根据标识判断是否补偿钟漂
                    psr_cnt[index] = 1;
                }
                else
                {
                    DOUBLE psr = (g_smooth ? p_chan_meas->psr_smooth : p_chan_meas->psr)
                        + p_chan_meas->compensate_meter * g_compensate;
                    g_rough_range[index] = g_rough_range[index] * (psr_cnt[index] / (psr_cnt[index] + 1.)) +
                        psr / (psr_cnt[index] + 1.);
                    psr_cnt[index]++;
                }
                if ((rtcm_signal_mask & cell_sig_mask) == 0)
                {
                    cell_sig_mask |= rtcm_signal_mask;
                    nsig++;
                }

                cell_all_mask[index][store_index] = i + 1;
                ncell++;
            }
        }
    }

    if (nsat == 0)  /// No satellite to encode
        return 0;

    S32 msm_number = GetMsmNumber(msm_type, rtcm_system);
    S32 epoch_time = GetGnssEpochTime(rtcm_system, p_pvt_meas);

    SplitMask split_mask;
    if ((nsat * nsig) <= 64)
    {
        split_mask.cell_sat_mask[0] = cell_sat_mask;
        split_mask.cell_sig_mask[0] = cell_sig_mask;
        split_mask.ncell[0] = ncell;
        split_mask.split_num = 1;
    }
    else
    {
        SplitMsm(rtcm_system, cell_sat_mask, cell_sig_mask, cell_all_mask, &split_mask);
    }

    S32 total_bytes = 0;

    //std::bitset<64> bitset_sat_mask;
    U64 bitset_sat_mask;
    //std::bitset<32> bitset_sig_mask;
    U32 bitset_sig_mask;
    U32 bitset_sat_mask_higher32;
    for (S32 p = 0; p < split_mask.split_num; p++)
    {
        bitset_sat_mask = split_mask.cell_sat_mask[p];
        bitset_sig_mask = split_mask.cell_sig_mask[p];
        ncell = split_mask.ncell[p];
        bitset_sat_mask_higher32 = bitset_sat_mask >> 32;
        //os_printf(" bitset_sat_mask:%x %x  bitset_sig_mask:%x\r\n",bitset_sat_mask_higher32,(U32)bitset_sat_mask,bitset_sig_mask);
        //nsat = bitset_sat_mask.count();
        nsat = BitCount64(bitset_sat_mask);
        //nsig = bitset_sig_mask.count();
        nsig = BitCount32(bitset_sig_mask);
        //os_printf("nsat %d nsig %d \r\n",nsat,nsig);
        U32 multiple_bit = (b_multiple || (p < (split_mask.split_num - 1))) ? 1 : 0;
        // header
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 0, 12, msm_number);         //!< DF002, Message number
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 12, 12, g_rtcm_station.reference_station_id);   //!< DF003, Reference station ID
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 24, 30, epoch_time);        //!< DF004(GPS), DF248(GAL), DF427(BDS), DF416+DF034(GLS)GNSS Epoch Time
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 54, 1, multiple_bit); //!< DF393, Multiple Message Bit
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 55, 3, 0);                  //!< DF409, IODS
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 58, 7, 0);                  //!< DF001, Reserved
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 65, 2, 0);                  //!< DF411, Clock Steering Indicator
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 67, 2, 0);                  //!< DF412, External Clock Indicator
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 69, 1, 0);                  //!< DF417, DFree Smoothing
        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 70, 3, 0);                  //!< DF418, Smoothing Interval
        for (S32 i = 0; i < 64; i++)
			SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 73 + i, 1, getU64IndexBit(bitset_sat_mask,i));//!< DF394, 反序填入

        for (S32 i = 0; i < 32; i++)
			SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 137 + i, 1, getU32IndexBit(bitset_sig_mask,i));	//!< DF395, 反序填入

        S32 cell_mask_index = 0;
        S32 msm_header_length = nsat * nsig + 169;
        S32 sat_cnt = 0, cell_cnt = 0;
        if (nsat && nsig)
        {
            for (S32 i = 0; i < 64; i++)
            {
                if (getU64IndexBit(bitset_sat_mask, i) == 0)
                    continue;
                BOOL b_sat_data = FALSE;
                for (S32 j = 0; j < GetMaxSupportRtcmSignal(rtcm_system); j++)
                {
                    if (getU32IndexBit(bitset_sig_mask, MapStoreIndexToRtcmSignalIndex(rtcm_system, j) - 1) == 0)
                        continue;

                    if (cell_all_mask[i][j])
                    {
                        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 169 + (cell_mask_index++), 1, 1);       //!< DF396, cell mask 1bit
                        const PvtChanMeas* p_chan_meas = &p_chan_meas_base[cell_all_mask[i][j] - 1];
                        if (b_sat_data == FALSE)
                        {
                            EncodeSatelliteData(msm_type, p_chan_meas, rtcm3_header_length + msm_header_length, sat_cnt, nsat);
                            b_sat_data = TRUE;
                        }
                        EncodeSignalData(msm_type, p_chan_meas, p_pvt_meas->clk_drift, rtcm3_header_length + msm_header_length + GetSatelliteDataLenForMsmType(msm_type) * nsat, cell_cnt, ncell);
                        cell_cnt++;
                    }
                    else
                    {
                        SetBits(g_rtcm_encode_data.buff, rtcm3_header_length + 169 + (cell_mask_index++), 1, 0);       //!< DF396, cell mask 1bit
                    }
                }
                sat_cnt++;
            }
        }
        g_rtcm_encode_data.msg_bit = 169 + sat_cnt * GetSatelliteDataLenForMsmType(msm_type) + cell_cnt * GetSignalDataLenForMsmType(msm_type) + cell_mask_index;
        ComposeRtcmMsg();
        g_rtcm_encode_data.buff += g_rtcm_encode_data.total_byte;
        total_bytes += g_rtcm_encode_data.total_byte;
    }

    return total_bytes;
}

BOOL HaveMultipleMsm(U8 curr_sys, const PvtMeas *p_pvt_meas, U32 system_flags)
{
    if (curr_sys == SYS_GPS)
    {
        if ((system_flags & (1 << SYS_BD3)) && (p_pvt_meas->msm_system_mask & 0x2))
            return TRUE;
        if ((system_flags & (1 << SYS_GLO)) && (p_pvt_meas->msm_system_mask & 0x4))
            return TRUE;
        if ((system_flags & (1 << SYS_GAL)) && (p_pvt_meas->msm_system_mask & 0x8))
            return TRUE;
        if ((system_flags & (1 << SYS_QZS)) && (p_pvt_meas->msm_system_mask & 0x10))
            return TRUE;
    }
    else if (curr_sys == SYS_BD3)
    {
        if ((system_flags & (1 << SYS_GLO)) && (p_pvt_meas->msm_system_mask & 0x4))
            return TRUE;
        if ((system_flags & (1 << SYS_GAL)) && (p_pvt_meas->msm_system_mask & 0x8))
            return TRUE;
        if ((system_flags & (1 << SYS_QZS)) && (p_pvt_meas->msm_system_mask & 0x10))
            return TRUE;
    }
    else if (curr_sys == SYS_GLO)
    {
        if ((system_flags & (1 << SYS_GAL)) && (p_pvt_meas->msm_system_mask & 0x8))
            return TRUE;
        if ((system_flags & (1 << SYS_QZS)) && (p_pvt_meas->msm_system_mask & 0x10))
            return TRUE;
    }
    else if (curr_sys == SYS_GAL)
    {
        if ((system_flags & (1 << SYS_QZS)) && (p_pvt_meas->msm_system_mask & 0x10))
            return TRUE;
    }
    return FALSE;
}

void EncodeRtcmMsm(const PvtMeas *p_pvt_meas, U8 *p_rtcm_buff, S32 msm_type, U8 system_mask, S32 *rtcm_len_out)
{
    g_rtcm_encode_data.buff = p_rtcm_buff;
    S32 rtcm_len = 0;

    U32 system_flags = GetValidSystemFlags(p_pvt_meas);
    system_flags &= system_mask;  // system_mask是外部传进来的，因此需要过滤

    if ((system_mask & (1 << SYS_GPS)) && (p_pvt_meas->msm_system_mask & 0x1))
    {
        rtcm_len += EncodeMsm(msm_type, SYS_GPS, p_pvt_meas, HaveMultipleMsm(SYS_GPS, p_pvt_meas, system_flags));
    }

    if ((system_mask & (1 << SYS_BD3)) && (p_pvt_meas->msm_system_mask & 0x2))
    {
        rtcm_len += EncodeMsm(msm_type, SYS_BD3, p_pvt_meas, HaveMultipleMsm(SYS_BD3, p_pvt_meas, system_flags));
    }

    if ((system_mask & (1 << SYS_GLO)) && (p_pvt_meas->msm_system_mask & 0x4))
    {
        rtcm_len += EncodeMsm(msm_type, SYS_GLO, p_pvt_meas, HaveMultipleMsm(SYS_GLO, p_pvt_meas, system_flags));
    }

    if ((system_mask & (1 << SYS_GAL)) && (p_pvt_meas->msm_system_mask & 0x8))
    {
        rtcm_len += EncodeMsm(msm_type, SYS_GAL, p_pvt_meas, HaveMultipleMsm(SYS_GAL, p_pvt_meas, system_flags));
    }

    if ((system_mask & (1 << SYS_QZS)) && (p_pvt_meas->msm_system_mask & 0x10))
    {
        rtcm_len += EncodeMsm(msm_type, SYS_QZS, p_pvt_meas, HaveMultipleMsm(SYS_QZS, p_pvt_meas, system_flags));
    }

    *rtcm_len_out = rtcm_len;
}

void EncodeGpsEph_1019(GpsEphemeris *eph, const GpsTgd *tgd)
{
    if ((eph->flag & EPHEMERIS_VALID) == 0)
        return;
    if (eph->eph_type == ModernGps)
    {
        return;     //!< 现代化星历按下述方式转换后精度无法保证，故不进行转换
        DOUBLE tk = 3600.;  //!< 按照星历每2小时更新一次，1小时时的外推值估算n和sqrtA，可靠性待查
        DOUBLE sqrt_gm = WGS_SQRT_GM;
        DOUBLE A_ref = 26559710.0;
        DOUBLE A0 = A_ref + eph->deltaA;
        eph->n = sqrt_gm / A0 / sqrt(A0) + eph->delta_n0 + 0.5 * eph->delta_n0dot * tk;
        eph->A = A0 + eph->Adot * tk;
        eph->sqrtA = sqrt(eph->A);
    }
    S32 prn = eph->svid - MIN_GPS_SVID + 1;                                  //!< uint6
    S32 week = eph->week % 1024;                                             //!< uint10
    S32 toe = eph->toe / 16;                                                 //!< uint16
    S32 toc = eph->toc / 16;                                                 //!< uint16
    U32 sqrtA = (U32) (eph->sqrtA / P2_19 + 0.5);                  //!< uint32
    U32 e = (U32) (eph->ecc / P2_33 + 0.5);                        //!< uint32
    S32 i0 = (S32) (round(eph->i0 / P2_31 / PI));                  //!< int32
    S32 omega0 = (S32) (round(eph->omega0 / P2_31 / PI));          //!< int32
    S32 w = (S32) (round(eph->w / P2_31 / PI));                    //!< int32
    S32 M0 = (S32) (round(eph->M0 / P2_31 / PI));                  //!< int32
    S32 delta_n = (S32) (round(eph->delta_n / P2_43 / PI));        //!< int16
    S32 idot = (S32) (round(eph->idot / P2_43 / PI));              //!< int14
    S32 omega_dot = (S32) (round(eph->omega_dot / P2_43 / PI));    //!< int24
    S32 crs = (S32) (round(eph->crs / P2_5));                      //!< int16
    S32 crc = (S32) (round(eph->crc / P2_5));                      //!< int16
    S32 cus = (S32) (round(eph->cus / P2_29));                     //!< int16
    S32 cuc = (S32) (round(eph->cuc / P2_29));                     //!< int16
    S32 cis = (S32) (round(eph->cis / P2_29));                     //!< int16
    S32 cic = (S32) (round(eph->cic / P2_29));                     //!< int16
    S32 af0 = (S32) (round(eph->af0 / P2_31));                     //!< int22
    S32 af1 = (S32) (round(eph->af1 / P2_43));                     //!< int16
    S32 af2 = (S32) (round(eph->af2 / P2_55));                     //!< int8
    S32 tgd_ = (S32) (round(tgd->tgd / P2_31));                     //!< int8

    S32 i = rtcm3_header_length;
    SetBits(g_rtcm_encode_data.buff, i, 12, 1019); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 6, prn); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 10, week); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 4, eph->urai); i += 4;
    SetBits(g_rtcm_encode_data.buff, i, 2, eph->CAonL2); i += 2;
    SetBits(g_rtcm_encode_data.buff, i, 14, idot); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 8, eph->iode2); i += 8;
    SetBits(g_rtcm_encode_data.buff, i, 16, toc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 8, af2); i += 8;
    SetBits(g_rtcm_encode_data.buff, i, 16, af1); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 22, af0); i += 22;
    SetBits(g_rtcm_encode_data.buff, i, 10, eph->iodc); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 16, crs); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 16, delta_n); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, M0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cuc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, e); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cus); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, sqrtA); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, toe); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 16, cic); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, omega0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cis); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, i0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, crc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, w); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 24, omega_dot); i += 24;
    SetBits(g_rtcm_encode_data.buff, i, 8, tgd_); i += 8;
    SetBits(g_rtcm_encode_data.buff, i, 6, eph->health); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 1, eph->L2PDataFlag); i += 1;
    SetBits(g_rtcm_encode_data.buff, i, 1, eph->fitInterval); i += 1;
    // total
    g_rtcm_encode_data.msg_bit = i - rtcm3_header_length;
    ComposeRtcmMsg();
}

void EncodeQzssEph_1044(GpsEphemeris *eph, const GpsTgd *tgd)
{
    if ((eph->flag & EPHEMERIS_VALID) == 0)
        return;
    if (eph->eph_type == ModernGps)
    {
        return;     //!< 现代化星历按下述方式转换后精度无法保证，故不进行转换
        DOUBLE tk = 3600.;  //!< 按照星历每2小时更新一次，1小时时的外推值估算n和sqrtA，可靠性待查
        DOUBLE sqrt_gm = WGS_SQRT_GM;
        DOUBLE A_ref = 26559710.0;
        DOUBLE A0 = A_ref + eph->deltaA;
        eph->n = sqrt_gm / A0 / sqrt(A0) + eph->delta_n0 + 0.5 * eph->delta_n0dot * tk;
        eph->A = A0 + eph->Adot * tk;
        eph->sqrtA = sqrt(eph->A);
    }
    S32 prn = eph->svid - MIN_QZSS_SVID + 1;                                 //!< uint6
    S32 week = eph->week % 1024;                                             //!< uint10
    S32 toe = eph->toe / 16;                                                 //!< uint16
    S32 toc = eph->toc / 16;                                                 //!< uint16
    U32 sqrtA = (U32) (eph->sqrtA / P2_19 + 0.5);                  //!< uint32
    U32 e = (U32) (eph->ecc / P2_33 + 0.5);                        //!< uint32
    S32 i0 = (S32) (round(eph->i0 / P2_31 / PI));                  //!< int32
    S32 omega0 = (S32) (round(eph->omega0 / P2_31 / PI));          //!< int32
    S32 w = (S32) (round(eph->w / P2_31 / PI));                    //!< int32
    S32 M0 = (S32) (round(eph->M0 / P2_31 / PI));                  //!< int32
    S32 delta_n = (S32) (round(eph->delta_n / P2_43 / PI));        //!< int16
    S32 idot = (S32) (round(eph->idot / P2_43 / PI));              //!< int14
    S32 omega_dot = (S32) (round(eph->omega_dot / P2_43 / PI));    //!< int24
    S32 crs = (S32) (round(eph->crs / P2_5));                      //!< int16
    S32 crc = (S32) (round(eph->crc / P2_5));                      //!< int16
    S32 cus = (S32) (round(eph->cus / P2_29));                     //!< int16
    S32 cuc = (S32) (round(eph->cuc / P2_29));                     //!< int16
    S32 cis = (S32) (round(eph->cis / P2_29));                     //!< int16
    S32 cic = (S32) (round(eph->cic / P2_29));                     //!< int16
    S32 af0 = (S32) (round(eph->af0 / P2_31));                     //!< int22
    S32 af1 = (S32) (round(eph->af1 / P2_43));                     //!< int16
    S32 af2 = (S32) (round(eph->af2 / P2_55));                     //!< int8
    S32 tgd_ = (S32) (round(tgd->tgd / P2_31));                     //!< int8

    S32 i = rtcm3_header_length;
    SetBits(g_rtcm_encode_data.buff, i, 12, 1044); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 4, prn); i += 4;
    SetBits(g_rtcm_encode_data.buff, i, 16, toc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 8, af2); i += 8;
    SetBits(g_rtcm_encode_data.buff, i, 16, af1); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 22, af0); i += 22;
    SetBits(g_rtcm_encode_data.buff, i, 8, eph->iode2); i += 8;
    SetBits(g_rtcm_encode_data.buff, i, 16, crs); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 16, delta_n); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, M0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cuc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, e); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cus); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, sqrtA); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, toe); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 16, cic); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, omega0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cis); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, i0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, crc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, w); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 24, omega_dot); i += 24;
    SetBits(g_rtcm_encode_data.buff, i, 14, idot); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 2, eph->CAonL2); i += 2;
    SetBits(g_rtcm_encode_data.buff, i, 10, week); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 4, eph->urai); i += 4;
    SetBits(g_rtcm_encode_data.buff, i, 6, eph->health); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 8, tgd_); i += 8;
    SetBits(g_rtcm_encode_data.buff, i, 10, eph->iodc); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 1, eph->fitInterval); i += 1;
    // total
    g_rtcm_encode_data.msg_bit = i - rtcm3_header_length;
    ComposeRtcmMsg();
}

void EncodeBdsEph_1042(BdsEphemeris *eph, const BdsTgd *tgd)
{
    if ((eph->flag & EPHEMERIS_VALID) == 0)
        return;
    if (eph->eph_type == ModernBds)
    {
        return;     //!< 现代化星历按下述方式转换后精度无法保证，故不进行转换
        DOUBLE tk = 1800.;  //!< 按照星历每2小时更新一次，1小时时的外推值估算n和sqrtA，可靠性待查
        DOUBLE sqrt_gm = CGS2000_SQRT_GM;
        DOUBLE A_ref = (eph->sat_type == MEO) ? 27906100.0 : 42162200.0;
        DOUBLE A0 = A_ref + eph->deltaA;
        eph->n = sqrt_gm / A0 / sqrt(A0) + eph->delta_n0 + 0.5 * eph->delta_n0dot * tk;
        eph->A = A0 + eph->Adot * tk;
        eph->sqrtA = sqrt(eph->A);
    }
    S32 prn = eph->svid - MIN_BDS_SVID + 1;  //!< uint6
    S32 week = eph->week;                    //!< uint13
    S32 toe = eph->toe / 8;                  //!< uint17
    S32 toc = eph->toc / 8;                  //!< uint17
    U32 sqrtA = (U32) (eph->sqrtA / P2_19 + 0.5);                  //!< uint32
    U32 e = (U32) (eph->ecc / P2_33 + 0.5);                        //!< uint32
    S32 i0 = (S32) (round(eph->i0 / P2_31 / PI));                  //!< int32
    S32 omega0 = (S32) (round(eph->omega0 / P2_31 / PI));          //!< int32
    S32 w = (S32) (round(eph->w / P2_31 / PI));                    //!< int32
    S32 M0 = (S32) (round(eph->M0 / P2_31 / PI));                  //!< int32
    S32 delta_n = (S32) (round(eph->delta_n / P2_43 / PI));        //!< int16
    S32 idot = (S32) (round(eph->idot / P2_43 / PI));              //!< int14
    S32 omega_dot = (S32) (round(eph->omega_dot / P2_43 / PI));    //!< int24
    S32 crs = (S32) (round(eph->crs / P2_6));                      //!< int18
    S32 crc = (S32) (round(eph->crc / P2_6));                      //!< int18
    S32 cus = (S32) (round(eph->cus / P2_31));                     //!< int18
    S32 cuc = (S32) (round(eph->cuc / P2_31));                     //!< int18
    S32 cis = (S32) (round(eph->cis / P2_31));                     //!< int18
    S32 cic = (S32) (round(eph->cic / P2_31));                     //!< int18
    S32 af0 = (S32) (round(eph->af0 / P2_33));                     //!< int24
    S32 af1 = (S32) (round(eph->af1 / P2_50));                     //!< int22
    S32 af2 = (S32) (round(eph->af2 / P2_66));                     //!< int11
    S32 tgd1 = (S32) (round(tgd->tgd_B1I * 1e10));                 //!< int10
    S32 tgd2 = (S32) (round(tgd->tgd_B2I * 1e10));                 //!< int10

    S32 i = rtcm3_header_length;
    SetBits(g_rtcm_encode_data.buff, i, 12, 1042); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 6, prn); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 13, week); i += 13;
    SetBits(g_rtcm_encode_data.buff, i, 4, eph->urai); i += 4;
    SetBits(g_rtcm_encode_data.buff, i, 14, idot); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 5, eph->iode2); i += 5;
    SetBits(g_rtcm_encode_data.buff, i, 17, toc); i += 17;
    SetBits(g_rtcm_encode_data.buff, i, 11, af2); i += 11;
    SetBits(g_rtcm_encode_data.buff, i, 22, af1); i += 22;
    SetBits(g_rtcm_encode_data.buff, i, 24, af0); i += 24;
    SetBits(g_rtcm_encode_data.buff, i, 5, eph->iodc); i += 5;
    SetBits(g_rtcm_encode_data.buff, i, 18, crs); i += 18;
    SetBits(g_rtcm_encode_data.buff, i, 16, delta_n); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, M0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 18, cuc); i += 18;
    SetBits(g_rtcm_encode_data.buff, i, 32, e); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 18, cus); i += 18;
    SetBits(g_rtcm_encode_data.buff, i, 32, sqrtA); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 17, toe); i += 17;
    SetBits(g_rtcm_encode_data.buff, i, 18, cic); i += 18;
    SetBits(g_rtcm_encode_data.buff, i, 32, omega0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 18, cis); i += 18;
    SetBits(g_rtcm_encode_data.buff, i, 32, i0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 18, crc); i += 18;
    SetBits(g_rtcm_encode_data.buff, i, 32, w); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 24, omega_dot); i += 24;
    SetBits(g_rtcm_encode_data.buff, i, 10, tgd1); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 10, tgd2); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 1, eph->health); i += 1;
    // total
    g_rtcm_encode_data.msg_bit = i - rtcm3_header_length;
    ComposeRtcmMsg();
}

void EncodeGalEphFNav_1045(GalEphemeris *eph, const GalTgd *tgd)
{
    if ((eph->flag & EPHEMERIS_VALID) == 0)
        return;
    if (eph->eph_type != GalFNav)
        return;
    S32 prn = eph->svid - MIN_GAL_SVID + 1;  //!< uint6
    S32 week = eph->week;                    //!< uint12
    S32 idot = (S32) (round(eph->idot / P2_43 / PI));              //!< int14
    S32 toc = eph->toc / 60;                  //!< uint14
    S32 af2 = (S32) (round(eph->af2 / P2_59));                     //!< int6
    S32 af1 = (S32) (round(eph->af1 / P2_46));                     //!< int21
    S32 af0 = (S32) (round(eph->af0 / P2_34));                     //!< int31
    S32 crs = (S32) (round(eph->crs / P2_5));                      //!< int16
    S32 delta_n = (S32) (round(eph->delta_n / P2_43 / PI));        //!< int16
    S32 M0 = (S32) (round(eph->M0 / P2_31 / PI));                  //!< int32
    S32 cuc = (S32) (round(eph->cuc / P2_29));                     //!< int16
    U32 e = (U32) (eph->ecc / P2_33 + 0.5);                        //!< uint32
    S32 cus = (S32) (round(eph->cus / P2_29));                     //!< int16
    U32 sqrtA = (U32) (eph->sqrtA / P2_19 + 0.5);                  //!< uint32
    S32 toe = eph->toe / 60;                                                 //!< uint14
    S32 cic = (S32) (round(eph->cic / P2_29));                     //!< int16
    S32 omega0 = (S32) (round(eph->omega0 / P2_31 / PI));          //!< int32
    S32 cis = (S32) (round(eph->cis / P2_29));                     //!< int16
    S32 i0 = (S32) (round(eph->i0 / P2_31 / PI));                  //!< int32
    S32 crc = (S32) (round(eph->crc / P2_5));                      //!< int16
    S32 w = (S32) (round(eph->w / P2_31 / PI));                    //!< int32
    S32 omega_dot = (S32) (round(eph->omega_dot / P2_43 / PI));    //!< int24
    S32 bgd_e5a_e1 = (S32) (round(tgd->bgd_E1E5a / P2_32));        //!< int10
    U32 health_oshs = eph->health & 0x3;
    U32 data_valid_status_osdvs = (eph->health >> 2) & 0x1;

    S32 i = rtcm3_header_length;
    SetBits(g_rtcm_encode_data.buff, i, 12, 1045); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 6, prn); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 12, week); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 10, eph->iodc); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 8, eph->urai); i += 8;
    SetBits(g_rtcm_encode_data.buff, i, 14, idot); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 14, toc); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 6, af2); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 21, af1); i += 21;
    SetBits(g_rtcm_encode_data.buff, i, 31, af0); i += 31;
    SetBits(g_rtcm_encode_data.buff, i, 16, crs); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 16, delta_n); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, M0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cuc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, e); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cus); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, sqrtA); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 14, toe); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 16, cic); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, omega0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cis); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, i0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, crc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, w); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 24, omega_dot); i += 24;
    SetBits(g_rtcm_encode_data.buff, i, 10, bgd_e5a_e1); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 2, health_oshs); i += 2;
    SetBits(g_rtcm_encode_data.buff, i, 1, data_valid_status_osdvs); i += 1;
    // total
    g_rtcm_encode_data.msg_bit = i - rtcm3_header_length;
    ComposeRtcmMsg();
}

void EncodeGalEphINav_1046(const GalEphemeris *eph, const GalTgd *tgd)
{
    if ((eph->flag & EPHEMERIS_VALID) == 0)
        return;
    if (eph->eph_type != GalINav)
        return;
    S32 prn = eph->svid - MIN_GAL_SVID + 1;  //!< uint6
    S32 week = eph->week;                    //!< uint12
    S32 idot = (S32) (round(eph->idot / P2_43 / PI));              //!< int14
    S32 toc = eph->toc / 60;                  //!< uint14
    S32 af2 = (S32) (round(eph->af2 / P2_59));                     //!< int6
    S32 af1 = (S32) (round(eph->af1 / P2_46));                     //!< int21
    S32 af0 = (S32) (round(eph->af0 / P2_34));                     //!< int31
    S32 crs = (S32) (round(eph->crs / P2_5));                      //!< int16
    S32 delta_n = (S32) (round(eph->delta_n / P2_43 / PI));        //!< int16
    S32 M0 = (S32) (round(eph->M0 / P2_31 / PI));                  //!< int32
    S32 cuc = (S32) (round(eph->cuc / P2_29));                     //!< int16
    U32 e = (U32) (eph->ecc / P2_33 + 0.5);                        //!< uint32
    S32 cus = (S32) (round(eph->cus / P2_29));                     //!< int16
    U32 sqrtA = (U32) (eph->sqrtA / P2_19 + 0.5);                  //!< uint32
    S32 toe = eph->toe / 60;                                                 //!< uint14
    S32 cic = (S32) (round(eph->cic / P2_29));                     //!< int16
    S32 omega0 = (S32) (round(eph->omega0 / P2_31 / PI));          //!< int32
    S32 cis = (S32) (round(eph->cis / P2_29));                     //!< int16
    S32 i0 = (S32) (round(eph->i0 / P2_31 / PI));                  //!< int32
    S32 crc = (S32) (round(eph->crc / P2_5));                      //!< int16
    S32 w = (S32) (round(eph->w / P2_31 / PI));                    //!< int32
    S32 omega_dot = (S32) (round(eph->omega_dot / P2_43 / PI));    //!< int24
    S32 bgd_e5a_e1 = (S32) (round(tgd->bgd_E1E5a / P2_32));        //!< int10
    S32 bgd_e5b_e1 = (S32) (round(tgd->bgd_E1E5b / P2_32));        //!< int10
    U32 e5b_signal_health = (eph->health >> 3) & 0x3;
    U32 e5b_data_valid = (eph->health >> 5) & 0x1;
    U32 e1b_signal_health = eph->health & 0x3;
    U32 e1b_data_valid = (eph->health >> 2) & 0x1;

    S32 i = rtcm3_header_length;
    SetBits(g_rtcm_encode_data.buff, i, 12, 1046); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 6, prn); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 12, week); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 10, eph->iodc); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 8, eph->urai); i += 8;
    SetBits(g_rtcm_encode_data.buff, i, 14, idot); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 14, toc); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 6, af2); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 21, af1); i += 21;
    SetBits(g_rtcm_encode_data.buff, i, 31, af0); i += 31;
    SetBits(g_rtcm_encode_data.buff, i, 16, crs); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 16, delta_n); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, M0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cuc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, e); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cus); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, sqrtA); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 14, toe); i += 14;
    SetBits(g_rtcm_encode_data.buff, i, 16, cic); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, omega0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, cis); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, i0); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 16, crc); i += 16;
    SetBits(g_rtcm_encode_data.buff, i, 32, w); i += 32;
    SetBits(g_rtcm_encode_data.buff, i, 24, omega_dot); i += 24;
    SetBits(g_rtcm_encode_data.buff, i, 10, bgd_e5a_e1); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 10, bgd_e5b_e1); i += 10;
    SetBits(g_rtcm_encode_data.buff, i, 2, e5b_signal_health); i += 2;
    SetBits(g_rtcm_encode_data.buff, i, 1, e5b_data_valid); i += 1;
    SetBits(g_rtcm_encode_data.buff, i, 2, e1b_signal_health); i += 2;
    SetBits(g_rtcm_encode_data.buff, i, 1, e1b_data_valid); i += 1;
    // total
    g_rtcm_encode_data.msg_bit = i - rtcm3_header_length;
    ComposeRtcmMsg();
}

void EncodeGlsEph_1020(const GlsEphemeris *eph)
{
    if ((eph->flag & EPHEMERIS_VALID) == 0)
        return;
    S32 prn = eph->svid - MIN_GLS_SVID + 1;  //!< uint6
    S32 fcn = eph->slot_freq + 7;
    S32 tb = eph->tb / 900;
    S32 x = (S32) (round(eph->x / P2_11 / 1e3));
    S32 y = (S32) (round(eph->y / P2_11 / 1e3));
    S32 z = (S32) (round(eph->z / P2_11 / 1e3));
    S32 vx = (S32) (round(eph->vx / P2_20 / 1e3));
    S32 vy = (S32) (round(eph->vy / P2_20 / 1e3));
    S32 vz = (S32) (round(eph->vz / P2_20 / 1e3));
    S32 ax = (S32) (round(eph->ax / P2_30 / 1e3));
    S32 ay = (S32) (round(eph->ay / P2_30 / 1e3));
    S32 az = (S32) (round(eph->az / P2_30 / 1e3));
    S32 gamma = (S32) (round(eph->gamma / P2_40));
    S32 tn = (S32) (round(eph->tn / P2_30));
    S32 dtn = (S32) (round(eph->dtn / P2_30));

    U32 Cn_valid_flag = 0;          //!< 为0表示下面Cn无效
    U32 Cn = 1;
    U32 additional_data_flag = 0;   //!< 为0标识下面Na/tc/N4/tgps/ln无效
    U32 Na = 0;
    S32 tc = 0;
    U32 N4 = 0;
    S32 tgps = 0;
    U32 ln = 0;

    S32 i = rtcm3_header_length;
    SetBits(g_rtcm_encode_data.buff, i, 12, 1020); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 6, prn); i += 6;
    SetBits(g_rtcm_encode_data.buff, i, 5, fcn); i += 5;
    SetBits(g_rtcm_encode_data.buff, i, 1, Cn); i += 1;                 //!< Cn默认为1
    SetBits(g_rtcm_encode_data.buff, i, 1, Cn_valid_flag); i += 1;                 //!< Cn Valid默认为0
    SetBits(g_rtcm_encode_data.buff, i, 4, eph->P & 0x3); i += 2;       //!<P1
    SetBits(g_rtcm_encode_data.buff, i, 12, eph->tk); i += 12;
    SetBits(g_rtcm_encode_data.buff, i, 1, eph->health); i += 1;        //!< Bn
    SetBits(g_rtcm_encode_data.buff, i, 1, (eph->P >> 2) & 0x1); i += 1;//!< P2 1bit
    SetBits(g_rtcm_encode_data.buff, i, 7, tb); i += 7;
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 24, vx); i += 24;       //!< intS24
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 27, x); i += 27;        //!< intS27
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 5, ax); i += 5;         //!< intS5
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 24, vy); i += 24;
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 27, y); i += 27;
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 5, ay); i += 5;
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 24, vz); i += 24;
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 27, z); i += 27;
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 5, az); i += 5;
    SetBits(g_rtcm_encode_data.buff, i, 1, (eph->P >> 3) & 0x1); i += 1;//!< P3 1bit
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 11, gamma); i += 11;    //!< intS11
    SetBits(g_rtcm_encode_data.buff, i, 3, (eph->P >> 5) & 0x3); i += 2;//!< P 2bit
    SetBits(g_rtcm_encode_data.buff, i, 1, (eph->P >> 7) & 0x1); i += 1;//!< ln 1bit
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 22, tn); i += 22;       //!< intS22
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 5, dtn); i += 5;        //!< intS5
    SetBits(g_rtcm_encode_data.buff, i, 5, eph->En); i += 5;            //!< uint5
    SetBits(g_rtcm_encode_data.buff, i, 1, (eph->P >> 4) & 0x1); i += 1;//!< P4 1bit
    SetBits(g_rtcm_encode_data.buff, i, 4, eph->Ft); i += 4;            //!< uint4
    SetBits(g_rtcm_encode_data.buff, i, 11, eph->Nt); i += 11;          //!< uint11
    SetBits(g_rtcm_encode_data.buff, i, 2, eph->M); i += 2;             //!< M 2bit
    SetBits(g_rtcm_encode_data.buff, i, 1, additional_data_flag); i += 1; //!< available of addition data 1bit
    SetBits(g_rtcm_encode_data.buff, i, 11, Na); i += 11;              //!< uint11
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 32, tc); i += 32;       //!< intS32
    SetBits(g_rtcm_encode_data.buff, i, 5, N4); i += 5;                //!< uint5
    SetBitsSignMag(g_rtcm_encode_data.buff, i, 22, tgps); i += 22;     //!< intS22
    SetBits(g_rtcm_encode_data.buff, i, 1, ln); i += 1;
    SetBits(g_rtcm_encode_data.buff, i, 7, 0); i += 7;
    // total
    g_rtcm_encode_data.msg_bit = i - rtcm3_header_length;
    ComposeRtcmMsg();
}

void EncodeRtcmEph(const void *p_eph, const void *p_tgd, U8 *p_rtcm_buff, S32 eph_type, S32 *rtcm_len_out)
{
    g_rtcm_encode_data.buff = p_rtcm_buff;
    g_rtcm_encode_data.total_byte = 0;
    if (eph_type == 1042)
        EncodeBdsEph_1042((BdsEphemeris *) p_eph, (BdsTgd *) p_tgd);    //!< 与63相同
    else if (eph_type == 1019)
        EncodeGpsEph_1019((GpsEphemeris *) p_eph, (GpsTgd *) p_tgd);
    else if (eph_type == 1020)
        EncodeGlsEph_1020((GlsEphemeris *) p_eph);
    else if (eph_type == 1045)
        EncodeGalEphFNav_1045((GpsEphemeris *) p_eph, (GalTgd *) p_tgd);
    else if (eph_type == 1046)
        EncodeGalEphINav_1046((GpsEphemeris *) p_eph, (GalTgd *) p_tgd);
    else if (eph_type == 1044)
        EncodeQzssEph_1044((GpsEphemeris *) p_eph, (GpsTgd *) p_tgd);
    *rtcm_len_out = g_rtcm_encode_data.total_byte;
}






