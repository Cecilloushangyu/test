#pragma once
#ifdef __cplusplus
extern "C" {
#endif

#include "ks_msg.h"
#include "ks_bb.h"
#include "middleware/nvm/ks_nvm.h"

typedef struct
{
    U8 com_count;
    U8 com1_uart_id;
    U8 com2_uart_id;
    U8 com3_uart_id;
    U8 com4_uart_id;
    U8 flash_page_id;
    U16 config_version;
    PARTITION_HANDLE flash_partition_handle;
    U32 rx_cfg[4];
    U32 rx_cfg_dual_antenna[4];
} PROTOCOL_CONFIG;

S32 ks_protocol_init();
S32 ks_protocol_config_get(PROTOCOL_CONFIG *config_content);
S32 ks_protocol_config_set(PROTOCOL_CONFIG *config_content);
S32 ks_protocol_default_config(const char *cmd, char *result_buffer, S32 buffer_size);
S32 ks_protocol_run();

S32 ks_protocol_execute(const char *cmd, char *result_buffer, S32 buffer_size);

typedef S32 (*PROTOCOL_KMD_CALLBACK)(char* param_string);
#define PROTOCOL_CALLBACK_SUCCESS 0
#define PROTOCOL_CALLBACK_BAD_PARAM 2
S32 ks_protocol_register_kmd(char* cmd, PROTOCOL_KMD_CALLBACK callback);
S32 ks_protocol_add_checksum(char* cmd_buffer);
S32 ks_protocol_print_string_in_kmd_callback(char* string);

typedef S32 (*PROTOCOL_MSG_CALLBACK)(char* print_buffer, S32* length_out, S32 max_buffer_size);
S32 ks_protocol_register_msg(char* msg, PROTOCOL_MSG_CALLBACK callback);

typedef S32 (*PROTOCOL_PARSER)(S32, char);
S32 ks_protocol_register_parser(PROTOCOL_PARSER parser);

#ifdef __cplusplus
}
#endif
