#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ks_bb.h"
#include "ks_msg.h"


void ks_sat_manager_enable(BOOL b_enable);
BOOL ks_sat_manager_get_eph(U8 system_id, S32 prn, PTR* p_eph);
BOOL ks_sat_manager_get_tgd(U8 system_id, S32 prn, PTR* p_tgd);
U64 ks_sat_manager_get_valid_flag(U8 system_id);
BOOL ks_sat_manager_get_el_az(U8 system, U8 prn, S16 *p_el, S16 *p_az);

void ks_sat_manager_reset();

// Used by middleware developing
void ks_sat_manager_enable_endpoint(BOOL b_enable, U32 endpoint_id);
typedef U32 SatUpdateHandle;
S32 ks_sat_manager_create_update_manager(SatUpdateHandle *p_update_handle);
U64 ks_sat_manager_get_update_flag(SatUpdateHandle update_handle, U8 system_id);
U64 ks_sat_manager_clear_update_flag(SatUpdateHandle update_handle, U8 system_id, U64 clear_flag);
S32 ks_sat_manager_reset_sat_update_flag_to_valid(SatUpdateHandle update_handle);
S32 ks_sat_manager_reset_sat_update_flag(SatUpdateHandle update_handle, U8 system_id);

#ifdef __cplusplus
}
#endif
