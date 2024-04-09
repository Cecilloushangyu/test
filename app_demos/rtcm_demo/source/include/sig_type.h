#pragma once
#include "ks_bb.h"
#include "ks_datatypes.h"
#include "pvt_const.h"

enum class SignalID : U8 {
    BDS_B1I = ID_BD3B1I,
    BDS_B1C = ID_BD3B1C,
    BDS_B1A = ID_BD3B1A,
    BDS_B2I = ID_BD3B2I,
    BDS_B2a = ID_BD3B2a,
    BDS_B2b = ID_BD3B2b,
    BDS_B3I = ID_BD3B3I,
    BDS_B3A = ID_BD3B3A,
    BDS_B3AE = ID_BD3B3AE,
    BDS_B3Q = ID_BD3B3Q,
    GPS_L1CA = ID_GPSL1CA,
    GPS_L2C = ID_GPSL2C,
    GPS_L5 = ID_GPSL5,
    GLS_G1 = ID_GLOG1,
	GLS_G2 = ID_GLOG2,
    GAL_E1 = ID_GALE1,
    GAL_E5a = ID_GALE5a,
	GAL_E5b = ID_GALE5b,
    QZSS_L1CA = ID_QZSL1CA,
    QZSS_L2C = ID_QZSL2C,
	QZSS_L5 = ID_QZSL5,
    NDEF = ID_NULL,
    ERR = ID_BadSignalID
};

enum class SystemID : U8 {
    BDS = SYS_BD3,
    GPS = SYS_GPS,
    GLS = SYS_GLO,
    GAL = SYS_GAL,
    QZSS = SYS_QZS,
    NDEF = 0xFF
};

inline SignalID MapSignalID(U8 signal_id)
{
    switch (signal_id)
    {
    case ID_GPSL1CA:
        return SignalID::GPS_L1CA;
    case ID_GPSL2C:
        return SignalID::GPS_L2C;
    case ID_GPSL5:
        return SignalID::GPS_L5;
    case ID_BD3B1C:
        return SignalID::BDS_B1C;
    case ID_BD3B1I:
        return SignalID::BDS_B1I;
    case ID_BD3B1A:
        return SignalID::BDS_B1A;
    case ID_BD3B2I:
        return SignalID::BDS_B2I;
    case ID_BD3B2a:
        return SignalID::BDS_B2a;
    case ID_BD3B2b:
        return SignalID::BDS_B2b;
    case ID_BD3B3I:
        return SignalID::BDS_B3I;
    case ID_BD3B3A:
        return SignalID::BDS_B3A;
    case ID_BD3B3AE:
        return SignalID::BDS_B3AE;
    case ID_BD3B3Q:
        return SignalID::BDS_B3Q;
    case ID_GLOG1:
        return SignalID::GLS_G1;
    case ID_GLOG2:
    	return SignalID::GLS_G2;
    case ID_GALE1:
        return SignalID::GAL_E1;
    case ID_GALE5a:
        return SignalID::GAL_E5a;
    case ID_GALE5b:
    	return SignalID::GAL_E5b;
    case ID_QZSL1CA:
        return SignalID::QZSS_L1CA;
    case ID_QZSL2C:
        return SignalID::QZSS_L2C;
    case ID_QZSL5:
    	return SignalID::QZSS_L5;
    default:
        /// 不支持的信号
        return SignalID::ERR;
    }
}

inline SystemID FindSystemID(U8 signal_id)
{
    switch (signal_id)
    {
    case ID_GPSL1CA:
    case ID_GPSL2C:
    case ID_GPSL5:      return SystemID::GPS;
    case ID_BD3B1C:
    case ID_BD3B1I:
    case ID_BD3B1A:
    case ID_BD3B2I:
    case ID_BD3B2a:
    case ID_BD3B2b:
    case ID_BD3B3I:
    case ID_BD3B3A:
    case ID_BD3B3AE:
    case ID_BD3B3Q:     return SystemID::BDS;
    case ID_GLOG1:
    case ID_GLOG2:
    					return SystemID::GLS;
    case ID_GALE1:
    case ID_GALE5a:
    case ID_GALE5b:     return SystemID::GAL;
    case ID_QZSL1CA:
    case ID_QZSL2C:
    case ID_QZSL5:      return SystemID::QZSS;
    default:
        /// 不支持的信号
        return SystemID::NDEF;
    }
}

inline SystemID FindSystemID(SignalID signal_id)
{
    switch (signal_id)
    {
    case SignalID::GPS_L1CA:
    case SignalID::GPS_L2C:
    case SignalID::GPS_L5:      return SystemID::GPS;
    case SignalID::BDS_B1I:
    case SignalID::BDS_B2I:
    case SignalID::BDS_B3I:
    case SignalID::BDS_B3Q:
    case SignalID::BDS_B1A:
    case SignalID::BDS_B3A:
    case SignalID::BDS_B3AE:
    case SignalID::BDS_B1C:
    case SignalID::BDS_B2a:
    case SignalID::BDS_B2b:     return SystemID::BDS;
    case SignalID::GLS_G1:
    case SignalID::GLS_G2:      return SystemID::GLS;
    case SignalID::GAL_E1:
    case SignalID::GAL_E5a:
    case SignalID::GAL_E5b:     return SystemID::GAL;
    case SignalID::QZSS_L1CA:
    case SignalID::QZSS_L2C:
    case SignalID::QZSS_L5:
    							return SystemID::QZSS;
    default: break;
    }
    return SystemID::NDEF;
}

inline DOUBLE GetSignalWaveLength(SignalID signal_id, S32 gls_freq = 0)
{
    switch (signal_id)
    {
    case SignalID::GPS_L1CA:    return GPS_L1CA_WAVELENGTH;
    case SignalID::GPS_L2C:     return GPS_L2_WAVELENGTH;
    case SignalID::GPS_L5:     return GPS_L5_WAVELENGTH;
    case SignalID::BDS_B1I:     return BDS_B1I_WAVELENGTH;
    case SignalID::BDS_B2I:     return BDS_B2I_WAVELENGTH;
    case SignalID::BDS_B3I:
    case SignalID::BDS_B3Q:     return BDS_B3IQ_WAVELENGTH;
    case SignalID::BDS_B1A:     return BDS_B1A_DOWN_WAVELENGTH; //!< 默认B1A工作在下边带模式
    case SignalID::BDS_B3A:
    case SignalID::BDS_B3AE:    return BDS_B3A_WAVELENGTH;
    case SignalID::BDS_B1C:     return BDS_B1C_WAVELENGTH;
    case SignalID::BDS_B2a:     return BDS_B2a_WAVELENGTH;
    case SignalID::BDS_B2b:     return BDS_B2b_WAVELENGTH;
    case SignalID::GLS_G1:      return LIGHTSPEED / (1602000000. + 562500. * gls_freq);
    case SignalID::GLS_G2:      return LIGHTSPEED / (1246000000. + 437500. * gls_freq);
    case SignalID::GAL_E1:      return GAL_E1_WAVELENGTH;
    case SignalID::GAL_E5a:      return GAL_E5a_WAVELENGTH;
    case SignalID::GAL_E5b:      return GAL_E5b_WAVELENGTH;
    case SignalID::QZSS_L1CA:   return QZSS_L1CA_WAVELENGTH;
    case SignalID::QZSS_L2C:    return QZSS_L2C_WAVELENGTH;
    case SignalID::QZSS_L5:    return QZSS_L5_WAVELENGTH;
    default: break;
    }
    return 0.0;
}
