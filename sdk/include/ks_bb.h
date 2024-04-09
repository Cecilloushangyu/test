#pragma once

#include "ks_datatypes.h"
#include "ks_os.h"
#include "ks_msg_types.h"

#ifdef __cplusplus
extern "C" {
#endif

//  GNSS signal ID
typedef enum
{
	ID_NULL = 0,
	ID_BD3B1I = 1,
	ID_GPSL1C = 2,
	ID_BD3B1C = 3,
	ID_BD3B1A = 4,
	ID_BD3B2a = 5,
	ID_BD3B2b = 6,
	ID_BD3B3I = 7,
	ID_BD3B3Q = 8,
	ID_BD3B3A = 9,
	ID_BD3B3AE = 10,
	ID_GPSL1CA = 11,
	ID_GPSL2C = 12,
	ID_GPSL5 = 13,
	ID_GALE1 = 14,
	ID_GALE5a = 15,
	ID_GLOG1 = 16,
	ID_GLOG2 = 17,
	ID_BD3S2C = 18,
	ID_BD3S2A = 19,
	ID_BD3S1 = 20,
	ID_BD3B2I = 21,
	ID_QZSL1CA = 22,
	ID_QZSL2C = 23,
	ID_QZSL5 = 24,
	ID_GALE5b = 25,
	ID_GALE6 = 26,
	ID_LBAND = 27,
	ID_QZSL6 = 28,
	ID_SBASL1 = 29,
	ID_SBASB1A = 30,
	ID_IRNSSL5 = 31,
	ID_SIGNAL_NUM = 32,
	ID_BadSignalID = 255

} GNSSSignalID;

typedef enum
{
    SYS_NULL = 0,
    SYS_BD3 = 1,
    SYS_GPS = 2,
    SYS_GLO = 3,
    SYS_GAL = 4,
	SYS_RD3 = 5,
	SYS_QZS = 6,
	SYS_IRNSS = 7,
	SYS_SBAS = 8
} GNSSSystem;

typedef enum{
    SEV_WAAS = 0,
    SEV_EGNOS = 1,
    SEV_MSAS = 2,
    SEV_GAGAN = 3,
    SEV_SDCM = 4,
    SEV_BDSBAS = 5,
    SEV_KASS = 6,
    SEV_ASECNA = 7,
    SEV_SPAN = 8,
    SEV_NDEF = 255
} SBASService;

typedef struct
{
    int sdk_protocol_level;
    char sdk_version[16];
    char fw_version[16];
    char ap_version[16];
} BBVersionInfo;

typedef struct
{
	U8 package_id;
	U8 package_version;
} PackageInfo;

typedef struct
{
    int sdk_protocol_level;
    char sdk_version[16];
    char fw_version[16];
    char ap_version[16];
    PackageInfo package_info[KS_SDK_PKG_TYPE_NUM];
} BBVersionInfoExt;

typedef struct
{
    U8 flag;
    U8 system_id;
    U16 reserved;
    S32 pps_week;
    DOUBLE pps_tow;
} PPSTime;

typedef struct
{
    U8 quality;
    U8 system_id;
    U16 reserved;
    S32 week;
    DOUBLE tow;
} GnssTime;

typedef enum
{
    RX_NULL,
    RX_L1,
    RX_L2_L5,
    RX_B3_L6,
    RX_S,
    RX_Lband,
    RX_L1_Dual,
    RX_L2_L5_Dual,
    RX_B3_L6_Dual
} RXConfig;

typedef enum
{
	BB_Misc_Flash,
	BB_Misc_Meas,
	BB_Misc_Mode
} BBMiscConfig;

typedef enum
{
	BB_NIC_NULL,
	BB_NIC_L1,
	BB_NIC_L2_L,
	BB_NIC_L2_H,
	BB_NIC_L5,
	BB_NIC_B3,
	BB_NIC_G1,
	BB_NIC_G2,
	BB_NIC_L1_Dual,
	BB_NIC_L2_L_Dual,
	BB_NIC_L2_H_Dual,
	BB_NIC_L5_Dual,
	BB_NIC_B3_Dual,
	BB_NIC_G1_Dual,
	BB_NIC_G2_Dual
} BBNicChannel;

#define KS_OK 0
#define KS_UNKNOWN_ERROR 0xFFFFFFFF
#define KS_BB_START_ERROR_FW_VERSION_NOT_MATCH 0xB01

S32 ks_bb_set_rx(RXConfig rx1_cfg, RXConfig rx2_cfg, RXConfig rx3_cfg, RXConfig rx4_cfg);
void ks_bb_misc_config(BBMiscConfig name, void *p_value);
void ks_bb_set_nic(BBNicChannel nic_channel[4]);
U32 ks_bb_start(U32 flag);
void ks_bb_stop(void);
void ks_bb_chip_soft_reset();
U32 ks_bb_is_started();
U32 ks_bb_is_heading();
void ks_bb_set_signal(U32 signal_en);
U32 ks_bb_get_signal();
void ks_bb_send_rtcm(U8* p_data, S32 len);
void ks_bb_set_nav_rate(U32 rate_hz);
U32 ks_bb_get_nav_rate();
void ks_bb_set_pps(S32 id, U32 flag, S32 period_us, S32 pulse_width_us, S32 delay_ns);
void ks_bb_get_pps(S32 id, U32 *p_flag, S32 *p_period_us, S32 *p_pulse_width_us, S32 *p_delay_ns);
void ks_bb_pps_irq_enable(IRQEntry entry);
void ks_bb_pps_irq_disable();
void ks_bb_pps_irq_enable_ex(S32 id, IRQEntry entry);
void ks_bb_pps_irq_disable_ex(S32 id);
void ks_bb_set_debug_on(U32 uart_port);
void ks_bb_set_debug_off();
S32 ks_bb_get_debug_port();
void ks_bb_get_version_info(BBVersionInfo* version_info);
void ks_bb_get_version_info_ext(BBVersionInfoExt* version_info);
void ks_bb_set_el_cutoff(U32 sys_mask, DOUBLE el_cutoff_deg);
DOUBLE ks_bb_get_el_cutoff_deg(S32 sys_id);
S32 ks_bb_get_rtc_time(U32 *p_gps_week, DOUBLE *p_gps_tow);
U64 ks_bb_get_chip_id();

void ks_bb_agnss_set_eph(U8 system_id, U32 prn, const void* p_eph, S32 len);
void ks_bb_agnss_set_alm(U8 system_id, U32 prn, const void* p_alm, S32 len);
void ks_bb_agnss_set_pos(DOUBLE lat, DOUBLE lon, DOUBLE hae);
void ks_bb_agnss_set_time(S32 gps_week, S32 gps_ms_count);
void ks_bb_agnss_set_tgd(U8 system_id, U32 prn, const void* p_tgd, S32 len);

void ks_bb_sbas_set_mask(U32 sbas_mask);

void ks_bb_enable_debug(BOOL b_enable);
void ks_bb_enable_wdt(BOOL b_enable, U32 period);
void ks_bb_feed_wdt();

void ks_bb_set_sat_mask(U8 system_id, U64 sat_mask);
U64 ks_bb_get_sat_mask(U8 system_index);

S32 ks_bb_set_lband(S32 channel, S32 carrier_freq, S32 speed, S32 enable);

S32 ks_bb_get_gnss_time(GnssTime *p_gnss_time);

S32 ks_bb_set_sat_freq_enable(GNSSSystem system_id, U64 sat_enable[]);
S32 ks_bb_get_sat_freq_enable(GNSSSystem system_id, U64 *p_sat_enable);
S32 ks_bb_set_bds_sat_type(U64 geo_list, U64 igso_list, U64 meo_list);
S32 ks_bb_get_bds_sat_type(U64 *geo_list, U64 *igso_list, U64 *meo_list);

S32 ks_bb_get_geoidal_separation(DOUBLE lat_degree, DOUBLE lon_degree, DOUBLE *p_geoidal_separation);

void BP_DEBUG_OUTPUT(const U8* p_data, S32 len) __attribute__((weak));

#ifdef __cplusplus
}
#endif

