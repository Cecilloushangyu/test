#ifndef DELOS_SDK_PLATFORM_KS_DEBUG_H
#define DELOS_SDK_PLATFORM_KS_DEBUG_H

#include "ks_datatypes.h"
#include "ks_os.h"
#include "ks_msg.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef S32 (*DEBUG_CALLBACK) (const U8 *msg, S32 length);

typedef enum {
    DEBUG_MSG_CLASS_BB_MSG = 0,
    DEBUG_MSG_CLASS_RTK = 1,
    DEBUG_MSG_CLASS_RAW = 2,
    DEBUG_MSG_CLASS_MONITOR = 3,
    DEBUG_MSG_CLASS_USER_START = 0x80,
} DEBUG_MSG_CLASS;

/**
 * Low-level interfaces. Currently supports msg_class up to 0xFF; msg_id up to 0xFF; raw_id up to 15.
 */
S32 ks_debug_init();
S32 ks_debug_register_msg(U32 msg_class, DEBUG_CALLBACK callback);
S32 ks_debug_deregister_msg(U32 msg_class);
S32 ks_debug_output_buffer(U32 msg_class, U32 msg_id, const U8* buffer, int length);
S32 ks_debug_output_string(U32 msg_class, U32 msg_id, const char* str);
S32 ks_debug_register_raw(U32 raw_id, DEBUG_CALLBACK callback);
S32 ks_debug_deregister_raw(U32 raw_id);
S32 ks_debug_output_buffer_raw(U32 raw_id, const U8 *buffer, int length);
S32 ks_debug_output_string_raw(U32 raw_id, const char *str);

/**
 * Terminals
 */
S32 debug_terminal_uart0(const U8 *msg, S32 length);
S32 debug_terminal_uart1(const U8 *msg, S32 length);
S32 debug_terminal_uart2(const U8 *msg, S32 length);
S32 debug_terminal_uart3(const U8 *msg, S32 length);

/**
 * BB Msg
 * Msg supported:
 *  - DCavNavPvtResult
 *  - DCavNavPvtMeas
 *  - DCavNavEph
 *  - DCavNavIono
 *  - DCavNavUtc
 *  - DCavNavTgd
 *  - DCavNavGrid
 *  - DCavNavHas
 */
S32 ks_debug_bb_msg_register_callback(DEBUG_CALLBACK callback);
S32 ks_debug_bb_msg_enable(BBMsgID bb_msg_id);
S32 ks_debug_bb_msg_disable(BBMsgID bb_msg_id);
S32 ks_debug_bb_msg_enable_all();
S32 ks_debug_bb_msg_disable_all();

/**
 * System monitor
 */
S32 ks_debug_monitor_register_callback(DEBUG_CALLBACK callback);
S32 ks_debug_monitor_enable();
S32 ks_debug_monitor_disable();

#ifdef __cplusplus
};
#endif

#endif //DELOS_SDK_PLATFORM_KS_DEBUG_H
