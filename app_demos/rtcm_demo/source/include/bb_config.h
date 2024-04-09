#pragma once
#include "ks_datatypes.h"
 /**
  * @name 卫星数定义
  * @{
  */
#define EXTEND_BDS_SAT  1
#define BDS_SAT_TYPE_SIMULATOR 0

#define MAX_GPS_SAT_NUM 37                 //!< 最大支持GPS卫星数
#if EXTEND_BDS_SAT
#define  MAX_BDS_SAT_NUM  63                 //!< 最大支持北斗卫星数
#else
#define  MAX_BDS_SAT_NUM  40                //!< 最大支持北斗卫星数
#endif
#define  MAX_GLS_SAT_NUM  28                 //!< 最大支持Glonass卫星数
#define  MAX_GAL_SAT_NUM  40                 //!< 最大支持Galileo卫星数
#define  MAX_QZSS_SAT_NUM  10                 //!< 最大支持QZSS卫星数
#define  MAX_SYS_SAT_NUM  MAX_BDS_SAT_NUM    //!< 各系统卫星数的最大值（考虑通过运算设定）
#define  MAX_ALL_SAT_NUM (MAX_GPS_SAT_NUM + MAX_BDS_SAT_NUM + MAX_GLS_SAT_NUM + MAX_GAL_SAT_NUM + MAX_QZSS_SAT_NUM)   //!< 总计卫星数

#define MAX_GLS_FREQ_NUM  14    //!< Glonass最大频点数

   /*! @} */

   /**
    * @name 卫星编号定义
    * @{
    */
#define MIN_GPS_SVID  1										//!< GPS SVID起始序号
#define MAX_GPS_SVID  (MIN_GPS_SVID + MAX_GPS_SAT_NUM - 1)	//!< GPS SVID结束序号
#define MIN_BDS_SVID  161									//!< BDS SVID起始序号
#define MAX_BDS_SVID  (MIN_BDS_SVID + MAX_BDS_SAT_NUM - 1)	//!< BDS SVID结束序号
#define MIN_GLS_SVID  38									//!< GLO SVID起始序号
#define MAX_GLS_SVID  (MIN_GLS_SVID + MAX_GLS_SAT_NUM - 1)	//!< GLO SVID结束序号
#define MIN_GAL_SVID  97									//!< GPS SVID起始序号
#define MAX_GAL_SVID  (MIN_GAL_SVID + MAX_GAL_SAT_NUM - 1)	//!< GPS SVID结束序号
#define MIN_QZSS_SVID  225									//!< QZSS SVID起始序号
#define MAX_QZSS_SVID  (MIN_QZSS_SVID + MAX_QZSS_SAT_NUM - 1)	//!< QZSS SVID结束序号
    /*! @} */



