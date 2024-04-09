#ifndef DELOS_SDK_PLATFORM_KS_NVM_H
#define DELOS_SDK_PLATFORM_KS_NVM_H

#include "ks_datatypes.h"
#include "ks_os.h"
#include "ks_flash.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MAX_FILE_SIZE 4096
#define FLASH_SECTOR_SIZE_KB 64

typedef void *PARTITION_HANDLE;

S32 ks_nvm_create_partition(U32 offset_KB, U32 size_KB, U32 max_file_count, PARTITION_HANDLE *p_partition_handle);
S32 ks_nvm_read(PARTITION_HANDLE partition_handle, U32 file_id, void *p_data, U32 max_data_size, U32 *len_out);
S32 ks_nvm_write(PARTITION_HANDLE partition_handle, U32 file_id, const void *p_data, U32 len);
S32 ks_nvm_delete(PARTITION_HANDLE partition_handle, U32 file_id);
S32 ks_nvm_format(PARTITION_HANDLE partition_handle);

#ifdef __cplusplus
};
#endif

#endif //DELOS_SDK_PLATFORM_KS_NVM_H
