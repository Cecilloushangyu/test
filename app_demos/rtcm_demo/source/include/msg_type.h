#pragma once
#include "ks_datatypes.h"

typedef struct
{
    int sdk_protocol_level;
    char sdk_version[16];
    char fw_version[16];
    char ap_version[16];
} BBMsgVersionInfo;
