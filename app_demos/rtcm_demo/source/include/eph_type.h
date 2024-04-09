#pragma once
#include "ks_datatypes.h"


/// 卫星类型
enum  SatType 
{
    Reserved = 0,   //!< 保留
    GEO,            //!< GEO卫星
    IGSO,           //!< IGSO卫星
    MEO             //!< MEO卫星
};

/// 星历类型
enum  EphType 
{
    Tradition = 0,  //!< 传统星历（北斗的D1 D2 D2Q电文，GPS的LNAVL电文，GLS的GNAV中解出的星历）
    ModernBds,      //!< 北斗新星历（ANAV，CNAV1，CNAV2，CNAV3中解出的星历）
    ModernGps,      //!< GPS新星历（CNAV中解出的星历）
    GalINav,        //!< GAL INAV解出的星历
    GalFNav         //!< GAL FNAV解出的星历
};

typedef struct SDK_MSG_Gps_Ephemeris GpsEphemeris,BdsEphemeris,GalEphemeris;
typedef struct SDK_MSG_Gls_Ephemeris GlsEphemeris;
typedef struct SDK_MSG_Sbas_Ephemeris SbasEphemeris;


/// 星历有效标识
#define EPHEMERIS_VALID  0x01        //!< 星历有效
#define EPHEMERIS_CONFIRMED  0x02	//!< 星历确认（卫星已参加定位或者收到重复星历）
#define EPHEMERIS_SRC_PRM  0x04	    //!< 星历来源为P码
#define EPHEMERIS_SRC_NAVBIT  0x08	//!< 从电文中获取的星历
#define EPHEMERIS_SRC_EXTERN  0x10   //!< 从外部注入的星历
#define EPHEMERIS_SRC_FLASH  0x20    //!< 从FLASH中获取的星历
#define EPHEMERIS_UPDATEFLASH  0x80	//!< 标记星历需要更新到FLASH


typedef struct SDK_MSG_Bds_Tgd BdsTgd;
typedef struct SDK_MSG_Gps_Tgd GpsTgd;
typedef struct SDK_MSG_Gal_Tgd GalTgd;
typedef struct SDK_MSG_NavIC_Tgd NavICTgd;

