#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ks_msg.h"

typedef enum
{
    NMEA_BIT_RMC = 0,
    NMEA_BIT_GGA = 1,
    NMEA_BIT_GLL = 2,
    NMEA_BIT_GSA = 3,
    NMEA_BIT_GSV = 4,
    NMEA_BIT_VTG = 5,
    NMEA_BIT_ZDA = 6,
    NMEA_BIT_GST = 7,
    NMEA_BIT_DHV = 8,
    NMEA_BIT_HDT = 10,
    NMEA_BIT_NTR = 11,
    NMEA_BIT_TRA = 12,
    NMEA_BIT_KSXT = 13,
} NmeaTypeBitMask;

void ks_nmea_enable(BOOL b_enable);
void ks_nmea_get_result(char* nmea_string, U64 nmea_mask);

typedef struct
{
    S32 threshold_enable;
    DOUBLE threshold_pos_error_std_enu;
    DOUBLE threshold_vel_error_std_enu;
	S32 nmea_version;
	DOUBLE heading_offset;
	DOUBLE pitch_offset;
} NMEA_CONFIG;

void ks_nmea_config_get(NMEA_CONFIG* config_content);
BOOL ks_nmea_config_set(NMEA_CONFIG* config_content);


#ifdef __cplusplus
}
#endif
