#pragma once
#include "ks_datatypes.h"
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct  _MessageHead
{
	uint16_t msg_id;
	uint16_t msg_len;
	uint16_t param;
	uint16_t result;
} MessageHead;


typedef struct  _MessageBuffer
{
    MessageHead msghead;
    uint8_t  buffer[0];
} __attribute__((packed))MessageBuffer;


typedef enum
{
	IDRtcmMsm5 = 0x10,
	IDRtcmEphBbs = 0x11,
	IDRtcmEphGps = 0x12,
	IDRtcmEphGls = 0x13,
	IDRtcmEphGal = 0x14,
	IDRtcmEphQzss = 0x15,
	IDCavNavPvtResult = 0x20,
	IDCavNavPvtMeas = 0x21,
	IDCavNavEph = 0x22,
	IDCavNavEoe = 0x23,
	IDCavNavIono = 0x24,
	IDCavNavUtc = 0x25,
    IDCavNavTgd = 0x26,
    IDCavNavTracking = 0x27,
    IDCavNavHeading = 0x28,
	IDCavNavGrid = 0x29,
	IDCavNavSatInfo = 0x2a,
	IDCavNavHas = 0x2b,
	IDCavNavPPPB2b = 0x2c,
	IDCavNavAlm = 0x2d,
	IDCavNavQzssL6 = 0x2e,
    IDBBStart = 0x30,
    IDVersion = 0x31,
	IDRawBBInfo = 0x32,
    IDFlashSync = 0x34,
    IDFlashOtp = 0x35,
    IDFlashOpRead = 0x36,
    IDFlashOpWrite = 0x37,
    IDFlashOpErase = 0x38,
	IDNicInfo = 0x39,
	IDLBandInfo = 0x3A,
	IDLBandData = 0x3B,
    IDThreadStatistics = 0x3C,
} BBMsgID;

typedef enum
{
	PriHigh = 0x0,
	PriMid = 0x1,
	PriLow = 0x2,
} BBMsgPri;
	

void ks_msg_enable(BBMsgID msg_id);
void ks_msg_disable(BBMsgID msg_id);

typedef void (*MsgCallback) (BBMsgID msg_id, const U8 *buffer, U32 msg_len);
void ks_msg_register_callback(BBMsgID msg_id, MsgCallback callback);
void ks_msg_deregister_callback(BBMsgID msg_id, MsgCallback callback);

void ks_msg_set_priority(BBMsgID msg_id, BBMsgPri priority);

#ifdef __cplusplus
}
#endif

