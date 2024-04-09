#pragma once
#include "ks_datatypes.h"
#include "ks_msg_types.h"

enum  PvtFixType
{
    None = 0,       //!< 无解算
    Lsq2D,          //!< 2D解算
    Lsq5Sat,        //!< 免时解算
    Lsq3D,          //!< 最小二乘解算
    Kalman,         //!< 卡尔曼解算
    Static          //!< 静态授时解算
};

typedef struct SDK_MSG_PvtResult PvtResult;


