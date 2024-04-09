#pragma once
#include "ks_datatypes.h"

#define KS_SDK_PKG_TYPE_NUM             13
#define SDK_MSG_PVT_MEAS_VERSION        1
#define SDK_MSG_PVT_RESULT_VERSION      1
#define SDK_MSG_EPH_VERSION             1
#define SDK_MSG_ALM_VERSION				1
#define SDK_MSG_IONO_PARAM_VERSION      1
#define SDK_MSG_UTC_PARAM_VERSION       1
#define SDK_MSG_TGD_PARAM_VERSION       2
#define SDK_MSG_IONO_GRID_VERSION       1
#define SDK_MSG_E6_HAS_VERSION          1
#define SDK_MSG_PPP_B2b_VERSION         1
#define SDK_MSG_QZSS_L6_VERSION         1
#define SDK_MSG_LBAND_INFO				1
#define SDK_MSG_LBAND_DATA				1

struct SDK_MSG_PvtChanMeas
{
    U8 bb_chan_id;                  //!< bb通道序号
    U8 signal;                      //!< 信号
    U8 prn;                         //!< prn号
    S8 slot_freq;                   //!< gls频点号-7~6
    S8 frame_id;                    //!< 当前子帧号
    U8 reserve;                     //!< 保留
    U16 state;                      //!< 状态
    DOUBLE psr;                     //!< 伪距，单位米
    DOUBLE psr_smooth;              //!< 平滑伪距
    DOUBLE adr;                     //!< 载波相位，单位米
    DOUBLE doppler_speed;           //!< 多普勒，单位米/秒
    DOUBLE compensate_meter;        //!< 钟差补偿距离
    U16 cn0;                        //!< 载噪比，0.01dB-Hz
    U16 pld;                        //!< 锁相环指示
    U32 lock_time_ms;               //!< 锁定时间
};        

struct SDK_MSG_PvtMeas
{
    U32 bb_tag;                     //!< tag计数    
    U8 antenna_index;               //!< 天线序号
    U8 msm_system_mask;             //!< msm播发系统标识(bit0-GPS,1-BDS,2-GLS,3-GAL,4-QZSS,5-IRNSS)
    U8 gps_time_state;              //!< gps时间质量
    U8 bds_time_state;              //!< bds时间质量        
    U8 gls_time_state;              //!< gls时间质量
    U8 gal_time_state;              //!< gal时间质量
    U8 irnss_time_state;            //!< irnss时间质量    
    S8 gps_time_adjust_ms;          //!< gps时间调整值（在1ms调整时调整到整频度，通常为0）    
    S8 bds_time_adjust_ms;          //!< bds时间调整值（在1ms调整时调整到整频度，通常为0）
    S8 gls_time_adjust_ms;          //!< gls时间调整值（在1ms调整时调整到整频度，通常为0）
    S8 gal_time_adjust_ms;          //!< gal时间调整值（在1ms调整时调整到整频度，通常为0）
    S8 irnss_time_adjust_ms;        //!< irnss时间调整值（在1ms调整时调整到整频度，通常为0）    
    S8 gls_leap_year;               //!< gls闰年计数
    S8 gls_day_in_week;             //!< gls周内天，从0起始，0~6
    S16 gls_day_number;             //!< gls4年内天计数    
    S16 gps_week;                   //!< gps周
    S16 bds_week;                   //!< bds周            
    S16 gal_week;                   //!< gal周
    S16 irnss_week;                 //!< irnss周
    S32 gps_local_time_ms;          //!< gps虚拟本地时    
    S32 bds_local_time_ms;          //!< bds虚拟本地时
    S32 gls_local_time_ms;          //!< gls虚拟本地时        
    S32 gal_local_time_ms;          //!< gal虚拟本地时
    S32 irnss_local_time_ms;        //!< irnss虚拟本地时                    
    DOUBLE gps_rcv_time;            //!< gps解算时间
    DOUBLE bds_rcv_time;            //!< bds解算时间
    DOUBLE gls_rcv_time;            //!< gls解算时间
    DOUBLE gal_rcv_time;            //!< gal解算时间
    DOUBLE irnss_rcv_time;          //!< irnss解算时间
    DOUBLE clk_drift;               //!< 解算钟漂，单位m/s
    U32 error_flag;                 //!< 错误码
    U16 reserve;                    //!< 保留
    U16 meas_count;                 //!< 通道观测量计数
    PTR meas_pointer;               //!< 通到观测量指针，用于变长结构体解析，meas_count个有效通道观测量
};        

struct SDK_MSG_PvtResult
{
    U32 bb_tag;                     //!< tag计数
    U32 interval_ms;	            //!< 解算间隔，单位ms
    U8 antenna_index;               //!< 天线序号
    U8 sv_number;                   //!< 主天线定位卫星数
    U8 sv_number_dual;              //!< 副天线定位卫星数（双天线模式使用）
    U8 pvt_fix_type;                //!< 定位类型
    U8 rtk_status;                  //!< RTK状态
    U8 heading_status;              //!< 航向状态
    U8 pos_status;                  //!< 位置状态
    U8 vel_status;                  //!< 速度状态
    U8 time_status;                 //!< 时间状态
    U8 drift_status;	            //!< 钟漂状态
    U8 time_source;                 //!< UTC使用的时间系统
    U8 leap_second_status;          //!< 闰秒状态
    S8 gps_leap_second;	            //!< gps闰秒
    S8 bds_leap_second;             //!< bds闰秒
    U8 year;                        //!< 年	（0xFF为无效，当前年为year+2000）
    U8 month;                       //!< 月
    U8 day;                         //!< 日
    U8 hour;                        //!< 时
    U8 minute;                      //!< 分	
    U8 second;                      //!< 秒
    U16 milli_second;               //!< 毫秒		    
    U16 rtk_station_id;             //!< RTK基站ID
    S32 age_ms;                     //!< RTK差分龄期，单位ms
    FLOAT height_mod;               //!< 高程异常
    FLOAT baseline_length_rtk;      //!< RTK基线长度
    FLOAT baseline_length_heading;  //!< 航向基线长度
    FLOAT heading_degree;           //!< 航向角，单位度
    FLOAT clk_drift;                //!< 钟漂，单位m/s
    DOUBLE pos_ecef_x;              //!< ECEF坐标X，单位m
    DOUBLE pos_ecef_y;              //!< ECEF坐标Y，单位m
    DOUBLE pos_ecef_z;              //!< ECEF坐标Z，单位m
    DOUBLE vel_ecef_x;              //!< ECEF速度X，单位m/s
    DOUBLE vel_ecef_y;              //!< ECEF速度Y，单位m/s
    DOUBLE vel_ecef_z;              //!< ECEF速度Z，单位m/s
    FLOAT std_pos_ecef_x;           //!< ECEF坐标X标准差，单位m
    FLOAT std_pos_ecef_y;           //!< ECEF坐标Y标准差，单位m
    FLOAT std_pos_ecef_z;           //!< ECEF坐标Z标准差，单位m
    FLOAT cov_pos_ecef[6];          //!< ECEF坐标协方差矩阵
    FLOAT std_vel_ecef_x;           //!< ECEF速度X标准差，单位m/s
    FLOAT std_vel_ecef_y;           //!< ECEF速度Y标准差，单位m/s
    FLOAT std_vel_ecef_z;           //!< ECEF速度Z标准差，单位m/s
    FLOAT cov_vel_ecef[6];          //!< ECEF速度协方差矩阵
    FLOAT std_heading_degree;       //!< 航向角标准差，单位度
    FLOAT hdop;                     //!< HDOP
    FLOAT vdop;                     //!< VDOP
    FLOAT tdop;                     //!< TDOP
};

struct SDK_MSG_Gps_Ephemeris
{
    U16 flag;                       //!< 有效标识
    U16 svid;                       //!< 卫星号
    U16 health;                     //!< 健康标识
    U16 urai;                       //!< 距离精度指示（对于北斗新星历为SISAIoc 5bits, SISAIoc1 3bits, SISAIoc2 3bits）    
    U8 iode2;                       //!< 第2子帧星历数据龄期
    U8 iode3;                       //!< 第3子帧星历数据龄期
    U8 sat_type;                    //!< BD3卫星类型SatType_Reserved-保留，SatType_GEO-BD3GEO,SatType_IGSO-BD3IGSO,SatType_MEO-BD3MEO
    U8 eph_type;                    //!< 星历类型type_tradition-L1CA,B1I,B3I,B3Q, type_modern-B1C,B1A,B3A,L1C   
    U8 CAonL2;                      //!< GPS专用，L2上播发CA指示
    U8 fitInterval;                 //!< GPS专用，拟合间隔
    U8 AODO;                        //!< GPS专用，AODO数据指示   
    U8 L2PDataFlag;                 //!< GPS和NavIC专用，GPS: L2P Data Flag,0:L2 P-Code dataOn,1:L2 P-Code Nav data Off; NavIC [3:0] 为spare 2 after i0 and spare 2 after idot
    U16 iodc;                       //!< 时钟数据龄期,GAL为IODNav
    U16 top;                        //!< 数据预测的周内时刻（北斗新星历使用）
    S32 week;                       //!< 电文周（未补偿模糊度）
    S32 toc;                        //!< 卫星钟差参数参考时间
    S32 toe;                        //!< 星历参考时间
    DOUBLE af0;                     //!< 卫星钟差0阶改正项
    DOUBLE af1;                     //!< 卫星钟差1阶改正项
    DOUBLE af2;                     //!< 卫星钟差2阶改正项
    DOUBLE sqrtA;                   //!< 半长轴平方根
    DOUBLE ecc;                     //!< 偏心率
    DOUBLE w;                       //!< 近地点幅角
    DOUBLE delta_n;                 //!< 卫星平均运动速率与计算值之差
    DOUBLE M0;                      //!< 参考时间平近点角
    DOUBLE omega0;                  //!< 按参考时间计算的升交点赤经
    DOUBLE omega_dot;               //!< 升交点赤经变化率
    DOUBLE i0;                      //!< 参考时间轨道倾角
    DOUBLE idot;                    //!< 轨道倾角变化率
    DOUBLE cuc;                     //!< 纬度幅角的余弦调和改正项的振幅
    DOUBLE cus;                     //!< 纬度幅角的正弦调和改正项的振幅
    DOUBLE crc;                     //!< 轨道半径的余弦调和改正项的振幅
    DOUBLE crs;                     //!< 轨道半径的正弦调和改正项的振幅
    DOUBLE cic;                     //!< 轨道倾角的余弦调和改正项的振幅
    DOUBLE cis;                     //!< 轨道倾角的正弦调和改正项的振幅
    DOUBLE deltaA;                  //!< BD3或L1C 半长轴基准差
    DOUBLE Adot;                    //!< BD3或L1C 半长轴变化率
    DOUBLE delta_n0;                //!< BD3或L1C delta_n基准
    DOUBLE delta_n0dot;             //!< BD3或L1C delta_n变化率
    /// 根据基本参数中计算出的参数，防止重复计算
    DOUBLE A;               //!< 半长轴
    DOUBLE n;               //!< 平近点角速度修正量
    DOUBLE root_ecc;        //!< sqrt(1-e^2)
    DOUBLE omega_t;         //!< toe时刻升交点经度
    DOUBLE omega_delta;	    //!< omega_dot - OMEGDOT（WGS或CGCS2000）
    DOUBLE Ek;              //!< 平近点角，在计算卫星位置时迭代计算，相对论修正中使用
};

struct SDK_MSG_Gls_Ephemeris
{
    U16 flag;                       //!< 有效标识
    U16 svid;                       //!< 卫星号
    U16 health;                     //!< 健康标识
    S8 slot_freq;                   //!< GLO频点，-7~+6
    U8 P;                           //!< 控制字段
    U8 M;                           //!< 卫星类型，1为GLONASS-M
    U8 Ft;                          //!< 距离精度指示，需查表
    U8 En;                          //!< 数据龄期，单位为天
    U8 reserve1;                    //!< 保留
    U16 tk;                         //!< 时间（用30s的计数表示）
    S16 Nt;                         //!< 4年内日
    S32 tb;                         //!< 参考时刻
    U32 reserve2;                   //!< 保留
    DOUBLE gamma;                   //!< 考虑万有引力和相对论效应的频率修正量
    DOUBLE tn;                      //!< 卫星钟差
    DOUBLE dtn;                     //!< G1和G2的延迟
    DOUBLE x;                       //!< x位置，m
    DOUBLE y;                       //!< y位置，m
    DOUBLE z;                       //!< z位置，m
    DOUBLE vx;                      //!< x速度，m/s
    DOUBLE vy;                      //!< y速度，m/s
    DOUBLE vz;                      //!< z速度，m/s
    DOUBLE ax;                      //!< x加速度，m/s2
    DOUBLE ay;                      //!< y加速度，m/s2
    DOUBLE az;                      //!< z加速度，m/s2
};

struct SDK_MSG_Sbas_Ephemeris
{
    U16 flag;                       //!< 有效标识
    U16 svid;                       //!< 卫星号
    U16 health;                     //!< 健康标识
    U8 URA;                         //!< 精度信息
    U8 service_id;                  //!< 服务商编号
    S32 t0;                         //!< 参考时间，天内秒
    U32 reserve;                    //!< 保留
    DOUBLE x;                       //!< x位置，m
    DOUBLE y;                       //!< y位置，m
    DOUBLE z;                       //!< z位置，m
    DOUBLE vx;                      //!< x速度，m/s
    DOUBLE vy;                      //!< y速度，m/s
    DOUBLE vz;                      //!< z速度，m/s
    DOUBLE ax;                      //!< x加速度，m/s2
    DOUBLE ay;                      //!< y加速度，m/s2
    DOUBLE az;                      //!< z加速度，m/s2
    DOUBLE aGf0;                    //!< 相对于BDSBAS SNT 的钟差
    DOUBLE aGf1;                    //!< 相对于BDSBAS SNT 的钟漂
};

struct SDK_MSG_IonoParam8
{
    DOUBLE a0;                      //!<  α0
    DOUBLE a1;                      //!<  α1
    DOUBLE a2;                      //!<  α2
    DOUBLE a3;                      //!<  α3
    DOUBLE b0;                      //!<  β0
    DOUBLE b1;                      //!<  β1
    DOUBLE b2;                      //!<  β2
    DOUBLE b3;                      //!<  β3
    U8 flag;                        //!<  有效标识
};

struct SDK_MSG_IonoParam9
{
    DOUBLE a1;                      //!< a1
    DOUBLE a2;                      //!< a2
    DOUBLE a3;                      //!< a3
    DOUBLE a4;                      //!< a4
    DOUBLE a5;                      //!< a5
    DOUBLE a6;                      //!< a6
    DOUBLE a7;                      //!< a7
    DOUBLE a8;                      //!< a8
    DOUBLE a9;                      //!< a9
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_IonoParam3
{
    DOUBLE a0;                      //!< a0
    DOUBLE a1;                      //!< a1
    DOUBLE a2;                      //!< a2
    U8 storm_flag;                  //!< 5bit valid表示5个区域的扰动标识
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_Bds_Utc_Param
{
    DOUBLE a0;                      //!< 系统时相对于UTC的钟差
    DOUBLE a1;                      //!< 系统时相对于UTC的钟速
    DOUBLE a2;                      //!< 系统时相对于UTC的钟漂率
    U16 wn;                         //!< UTC参数参考周
    U16 wnlsf;                      //!< 新闰秒生效的周计数，DN对应的整周计数模256  
    U32 tot;                        //!< UTC参数参考时刻
    S8 tls;                         //!< 旧闰秒值
    S8 tlsf;                        //!< 新闰秒值
    U8 dn;                          //!< 新闰秒生效的周内日计数，为0~6
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_Gps_Utc_Param
{
    DOUBLE a0;                      //!< 系统时相对于UTC的钟差
    DOUBLE a1;                      //!< 系统时相对于UTC的钟速
    DOUBLE a2;                      //!< 系统时相对于UTC的钟漂率
    U16 wn;                         //!< UTC参数参考周
    U16 wnlsf;                      //!< 新闰秒生效的周计数，DN对应的整周计数模256
    U32 tot;                        //!< UTC参数参考时刻
    S8 tls;                         //!< 旧闰秒值
    S8 tlsf;                        //!< 新闰秒值
    U8 dn;                          //!< 新闰秒生效的周内日计数，为1~7
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_Gls_Utc_Param
{
    DOUBLE b1;                      //!< 系统时相对于UTC的钟差
    DOUBLE b2;                      //!< 系统时相对于UTC的钟差
    DOUBLE tc;                      //!< τc GLO与UTC（SU）的时间差
    DOUBLE tg;                      //!< GPS与GLO的时间差的小数部分
    S32 dn;                         //!< 闰秒参数接收时刻的天计数（年计数*1461 + 天计数），GLO闰秒是在当前季度末根据KP判断是否有闰秒
    U8 kp;                          //!< 闰秒标识2bit，00-季度末无闰秒，01-季度末有+1闰秒，11-季度末有-1闰秒，10-其他
    U8 flag;                        //!< UTC参数有效标识，GLO在String5播发Tc，Tg，DN，在String14播发A0，A1，KP，故分为两个标识，见UTC_PARAM_GLO_宏定义
};

struct SDK_MSG_Gal_Utc_Param
{
    DOUBLE a0;                      //!< 系统时相对于UTC的钟差
    DOUBLE a1;                      //!< 系统时相对于UTC的钟速
    U16 wn;                         //!< UTC参数参考周
    U16 wnlsf;                      //!< 新闰秒生效的周计数，DN对应的整周计数模256
    U32 tot;                        //!< UTC参数参考时刻
    S8 tls;                         //!< 旧闰秒值
    S8 tlsf;                        //!< 新闰秒值
    U8 dn;                          //!< 新闰秒生效的周内日计数，为1~7
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_NavIC_Utc_Param
{
    DOUBLE a0;                      //!< 系统时相对于UTC的钟差
    DOUBLE a1;                      //!< 系统时相对于UTC的钟速
    DOUBLE a2;                      //!< 系统时相对于UTC的钟漂率
    U16 wn;                         //!< UTC参数参考周
    U16 wnlsf;                      //!< 新闰秒生效的周计数，DN对应的整周计数模256
    U32 tot;                        //!< UTC参数参考时刻
    S8 tls;                         //!< 旧闰秒值
    S8 tlsf;                        //!< 新闰秒值
    U8 dn;                          //!< 新闰秒生效的周内日计数，为1~7
    U8 flag;                        //!< 有效标识，放在最后
};

struct SDK_MSG_Bds_Tgd
{
    DOUBLE tgd_B1I;                 //!< B1I群延迟（秒）
    DOUBLE tgd_B2I;                 //!< B2I群延迟（秒）
    DOUBLE tgd_B1Cp;                //!< B1Cp群延迟（秒）
    DOUBLE tgd_B2ap;                //!< B2ap群延迟（秒）
    DOUBLE tgd_B2bI;                //!< B2bI群延迟（秒）
    DOUBLE tgd_B2bQ;                //!< B2bQ群延迟（秒）
    DOUBLE reserve1;                //!< 保留
    DOUBLE reserve2;                //!< 保留
    U16 flag;                       //!< 有效标识
};

struct SDK_MSG_Gps_Tgd
{
	DOUBLE tgd;                     //!< LNav基准群延迟（秒）
	DOUBLE isc_tgd;					//!< CNav基准群延迟（秒）
    DOUBLE isc_L1CA;                //!< L1CA群延迟修正（秒）
    DOUBLE isc_L2;                  //!< L2群延迟修正（秒）
    DOUBLE isc_L5I5;                //!< L5I5群延迟修正（秒）
    DOUBLE isc_L5Q5;                //!< L5Q5群延迟修正（秒）
    DOUBLE isc_L6;                  //!< 保留，用于扩展L6群延迟修正（秒）
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_Gal_Tgd
{
    DOUBLE bgd_E1E5a;               //!< E5a群延时修正（秒）
    DOUBLE bgd_E1E5b;               //!< E5b群延时修正（秒）
    DOUBLE cb_E6c;                  //!< E6c码偏差
    DOUBLE pb_E6c;                  //!< E6c载波相位偏差
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_NavIC_Tgd
{
    DOUBLE tgd;                     //!< 基准群延迟（秒）
    DOUBLE isc_L5;                  //!< L5群延迟修正（秒）
    DOUBLE isc_L1;                  //!< 保留，用于扩展L1群延迟修正（秒）
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_SbasIonoGridBody
{
    S16 lat;                        //!< 格点纬度
    S16 lon;                        //!< 格点经度
    U16 GIVEI;                      //!< 电离层GIVEI信息（1~15）
    U16 GIVD;                       //!< 电离层垂直延迟，比例因子0.125m
    U8 IODI;                        //!< 数据龄期（1~3）
    U8 flag;                        //!< 有效标识
};

struct SDK_MSG_SBAS_IONO_GRID
{
    U8 service_id;                  //!< SBAS服务商编号
    U8 grid_number;                 //!< 格点数量
    struct SDK_MSG_SbasIonoGridBody grid_body[128];//!< 格点结构体数组，最大128格点，有效值为grid_number
};

struct SDK_MSG_HasPageBody
{
    U8 data[53];                    //!< 每个数据块包含53字节
};

struct SDK_MSG_E6_HAS
{    
    U8 HASS;                        //!< HAS状态
    U8 MT;                          //!< 数据包类型
    U8 MID;                         //!< 数据包ID编号
    U8 MS;                          //!< 数据块数量
    U8 flag;                        //!< HAS数据包有效标识
    struct SDK_MSG_HasPageBody page_body[32];      //!< 数据包结构体数组，最大32，有效值为ms
};

struct SDK_MSG_PPP_B2b
{
	U8 prn;							//!< 北斗PRN号
	U8 rsv_flag;					//!< 星上播发的预留标识位，低6bit有效。bit5为0表示本星PPP服务可用，其他位含义预留
	U16 reserve;					//!< 保留
	U32 nav_data[16];				//!< 译码后的原始导航，486bit有效。nav_data[0]的最高位对应第1bit，nav_data[15]的高6位有效
};

struct SDK_MSG_QZSS_L6
{
	U8 prn;							//!< QZSS PRN号
	U8 flag;						//!< flag保留
	S16 cn0;						//!< 载噪比
	U8 nav_data[250];				//!< 解调电文
};

struct SDK_MSG_LBand_ChannelInfo
{
    U8 channel;        //    通道号，0~3
    U8 state;          //    通道状态，参考后表
    U8 speed;          //    速度等级，定义和API相同
    U8 PLD;            //    PLL锁定指示，0~99
    U16 CN0;           //    CN0，单位是0.01，除以100得到单位为dBHz的值
    U16 rsv0;          //    保留
    U32 carrier_freq;  //    载波频率，和设置值相同，单位是1KHz
    S32 doppler;       //    载波多普勒测量值，单位是0.01Hz
    U32 symbol_cnt;    //    已经连续解调的symbol计数。从进入PLL锁定开始计算，直到PLL锁定丢失时清零
    U32 rsv1;          //    保留
    U32 rsv2;          //    保留
    U32 rsv3;          //    保留
};

struct SDK_MSG_LBand_Info
{
	U32 bb_tag;
	U8 ch_num;
	U8 rsv[3];
	struct SDK_MSG_LBand_ChannelInfo ch_info[4];
};

struct SDK_MSG_LBand_Data
{
	U8 channel;
	U8 rsv[3];
	U32 data[127];
};

struct SDK_MSG_Gps_Almanac
{
    U16 flag;			//!< 有效标识，参见ALMANAC_宏定义，固定在第1个字节
    U16 svid;			//!< 卫星号，固定在第2个字节
    U16 health;			//!< 为兼容BD2（9bit健康标识），使用U16，固定在第3,4个字节
    U8 sat_type;   		//!< BD3卫星类型0-保留，1-BD3GEO,2-BD3IGSO,3-BD3MEO
    U8 reserve; 		//!< 保留
    S32 week;			//!< 电文周（未补偿模糊度）
    S32 toa;			//!< 历书参考时间
    DOUBLE sqrtA;		//!< 半长轴平方根
    DOUBLE ecc;			//!< 偏心率
    DOUBLE w;			//!< 近地点幅角
    DOUBLE M0;			//!< 参考时间平近点角
    DOUBLE omega0;		//!< 按参考时间计算的升交点赤经
    DOUBLE omega_dot;	//!< 升交点赤经变化率
    DOUBLE i0;			//!< 参考时间轨道倾角的改正量
    DOUBLE af0;			//!< 卫星钟差
    DOUBLE af1;			//!< 卫星钟速
    /// 根据基本参数中计算出的参数，防止重复计算
    DOUBLE A;			//!< 半长轴
    DOUBLE n;			//!< 平近点角速度修正量
    DOUBLE root_ecc;	//!< sqrt(1-e^2)
    DOUBLE omega_t;		//!< toa时刻升交点经度
    DOUBLE omega_delta;	//!< omega_dot - OMEGDOT（WGS或CGCS2000）
};

struct SDK_MSG_Gls_Almanac
{
	U16 flag;				//!< 有效标识，固定在第1个字节
	U16 svid;				//!< 卫星号，固定在第2个字节
	U16 health;				//!< 健康标识，1为不健康，固定在第3,4个字节
	S8 slotFreq;			//!< GLO频点，-7~+6
	U8 N4;					//!< 闰年计数，5bit
	U16 ND;					//!< 4年内天计数，11bit
	U16 reserve1;			//!< 保留
	U32 reserve2;			//!< 保留
	DOUBLE tao;				//!< 参考时刻的卫星钟差，单位秒
	DOUBLE t;				//!< 历书参考时刻，天内秒
	DOUBLE lambda;			//!< 升交点经度，PZ90.02坐标系下
	DOUBLE i;				//!< 轨道倾角改正量
	DOUBLE ecc;				//!< 偏心率
	DOUBLE w;				//!< 参考时刻近地点幅角
	DOUBLE dt;				//!< 星钟修正量，GLO ICD为Draconian period（均值为43200s）修正量
	DOUBLE dt_dot;			//!< 星钟修正量速率
	/// 根据基本参数中计算出的参数，防止重复计算
	DOUBLE A;				//!< 半长轴
	DOUBLE w_dot;			//!< 近地点幅角变化率
	DOUBLE n;				//!< 平近点角速度修正量
	DOUBLE root_ecc;		//!< sqrt(1-e^2)
	DOUBLE omega_delta;		//!< omega_dot - PZ90_OMEGDOTE
};

struct SDK_MSG_Sbas_Almanac
{
    U16 flag;		//!< 有效标识，固定在第1个字节
    U16 svid;		//!< 卫星号，固定在第2个字节
    U16 health;		//!< 健康标识，1为不健康，固定在第3,4个字节
    U16 dataid;		//!< 数据编号
    S32 t0;	        //!< 参考时间，天内秒
    U32 reserve;	//!< 保留
    DOUBLE x;		//!< x位置，m
    DOUBLE y;		//!< y位置，m
    DOUBLE z;		//!< z位置，m
    DOUBLE vx;		//!< x速度，m/s
    DOUBLE vy;		//!< y速度，m/s
    DOUBLE vz;		//!< z速度，m/s
};

