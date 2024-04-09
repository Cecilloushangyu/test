#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ks_msg.h"

typedef struct {
//	U32 work_mode;
    S32 max_age_ms;
//	S32 el_mask;
//	S32 cn0_mask;
//	U32 system_mask;
//	U32 signal_mask;
    U32 gps_signal[3];	//!< GPS(QZSS)参与RTK解算的频点，最多支持3频，默认为L1CA/L2C/L5
    U32 bds_signal[3];	//!< BDS参与RTK解算的频点，最多支持3频，默认为B1I/B2I/B3I
    U32 gls_signal[3];	//!< GLS参与RTK解算的频点，最多支持3频，默认为G1/G2/NULL
    U32 gal_signal[3];	//!< GAL参与RTK解算的频点，最多支持3频，默认为E1/E5a/E5b
} RTK_CONFIG;

void ks_rtk_init(S32 thread_priority);
void ks_rtk_enable(BOOL b_enable);
BOOL ks_rtk_rtcm_input(const U8* p_data, S32 len);
void ks_rtk_reset();
void ks_rtk_config_get(RTK_CONFIG* config_content);
void ks_rtk_config_set(RTK_CONFIG* config_content);
DOUBLE ks_rtk_elapsed_time();
U64 ks_rtk_get_thread_time();

BOOL ks_rtk_get_baseline(DOUBLE *p_east, DOUBLE *p_north, DOUBLE *p_up, DOUBLE *p_len);
void ks_rtk_get_sat_number(S32 antenna_index, S32* p_track_sats, S32* p_fix_sats, S32* p_multi_freq_fix_sats);
void ks_rtk_get_heading_course(DOUBLE* p_course, DOUBLE* p_pitch, S32* p_status);
void ks_rtk_get_heading_error_std(DOUBLE* p_heading_std, DOUBLE* p_pitch_std);

void ks_rtk_enable_debug(BOOL b_enable);
void ks_rtk_enable_monitor(BOOL b_enable);

void RTK_DEBUG_OUTPUT(const U8* p_data, S32 len) __attribute__((weak));
void RTK_MONITOR_BIN_OUTPUT(const U8* p_data, S32 len) __attribute__((weak));
void RTK_MONITOR_OUTPUT(const char* p_string) __attribute__((weak));

#ifdef __cplusplus
}
#endif
