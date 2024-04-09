#pragma once

#include "ks_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif


void ks_sys_soft_reset();
U64 ks_sys_get_chip_id();
void ks_sys_get_tn(char TN[12]);
S32 ks_sys_update_auth_pkg(const U8 *pkg_buffer, S32 size);

#ifdef __cplusplus
}
#endif
