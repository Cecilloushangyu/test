#pragma once
#include "ks_datatypes.h"
#include "eph_type.h"
#include "pvt_type.h"
#include "pvt_meas_type.h"


PvtMeas* MeasDataGet();
PvtResult* PvtResultGet();

GpsEphemeris* EphGpsGet(U8 prn);
BdsEphemeris* EphBdsGet(U8 prn);
GlsEphemeris* EphGlsGet(U8 prn);
GalEphemeris* EphGalGet(U8 prn);
GpsEphemeris* EphQzssGet(U8 prn);
GpsTgd *TgdGpsGet(U8 prn);
BdsTgd *TgdBdsGet(U8 prn);
GalTgd *TgdGalGet(U8 prn);
GpsTgd *TgdQzssGet(U8 prn);


U64 EphGpsGetUpdateFlag();
U64 EphBdsGetUpdateFlag();
U64 EphGlsGetUpdateFlag();
U64 EphGalGetUpdateFlag();
U64 EphQzssGetUpdateFlag();
void EphAllClearUpdateFlag();


