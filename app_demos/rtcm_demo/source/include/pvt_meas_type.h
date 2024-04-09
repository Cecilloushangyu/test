#pragma once
#include "ks_datatypes.h"

typedef struct SDK_MSG_PvtChanMeas PvtChanMeas;

#define PVT_CHAN_MEAS_STATE_PSR_VALID  0x1      //!< 伪距有效
#define PVT_CHAN_MEAS_STATE_DOPPLER_VALID  0x2  //!< 多普勒有效
#define PVT_CHAN_MEAS_STATE_ADR_VALID  0x4      //!< 载波相位有效
#define PVT_CHAN_MEAS_STATE_FIX_VALID  0x8      //!< 可以参与解算（实际参与解算或者(仅由于被sig_mask未参与解算且inno<30m))
#define PVT_CHAN_MEAS_STATE_EL_LOW  0x10        //!< 仰角较低
#define PVT_CHAN_MEAS_STATE_ALL  0xFF           //!< 所有状态
#define PVT_CHAN_MEAS_STATE_SET_PRI_VALID  0x7  //!< PRI上报条件
#define PVT_CHAN_MEAS_STATE_SET_RTCM_VALID  0x7 //!< RTCM上报条件
#define PVT_CHAN_MEAS_STATE_SET_RTK_VALID  0xB  //!< RTK使用条件，允许载波相位无效（仅使用伪距进行差分）

typedef struct SDK_MSG_PvtMeas PvtMeas;

#define PVT_MEAS_TIME_STATE_PREDICT  6   //!< 与TimeStatus对应
#define PVT_MEAS_TIME_STATE_ACCURATE  7  //!< 与TimeStatus对应
