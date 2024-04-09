#pragma once

#include "ks_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif

#define FLASH_SIZE_KB 2048

#define DEFAULT_FLASH_START_ADDR 1536

S32 ks_flash_otp_read(U32 region_id, void *p_data, S32 max_data_size, S32 *len_out);
S32 ks_flash_otp_write(U32 region_id, const void *p_data, S32 len);
S32 ks_flash_otp_get_lock_status(U32 region_id, U32 *p_status);
S32 ks_flash_otp_lock(U32 region_id, U32 magic_word);

S32 ks_flash_read(U32 offset, U32 size, void *user_buffer);
S32 ks_flash_write(U32 offset, U32 size, const void *user_buffer);
S32 ks_flash_erase(U32 offset, U32 size);
S32 ks_flash_write_protect_off(U32 offset, U32 size);
S32 ks_flash_write_protect_on(U32 offset, U32 size);


#ifdef __cplusplus
}
#endif

