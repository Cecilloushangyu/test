
#ifndef __KS_SHELL_H
#define __KS_SHELL_H

#ifdef __cplusplus
 extern "C" {
#endif

#include "dlist.h"
#include <stdint.h>
#include "ks_uart.h"



#define CMD_ERR_PARAMS_FORMAT      		(1)  // params format error
#define CMD_ERR_DEVICE_OP_FAILED   		(2)  // device operation error
#define CMD_ERR_PARAMS_NOT_ENOUGH     	(3)  // params NUMBER NOT RIGHT
#define CMD_ERR_PARAM_NOT_EXIST     	(4)
#define CMD_ERR_OK                		(0)

typedef struct cmd_proc_t{
	const char * cmd;
	S32 uart_id;
	S32 (*fn)(struct cmd_proc_t* ctx,S32 argc, char **argv );
	struct cmd_proc_t * next;
	const char * help;
}cmd_proc_t;


typedef void (*ShellPrintfCbk)(  U32 uart_id, U8 *p_data, S32 len);

typedef struct  ShellPrintfCbkListCb
{
	struct list_node  cb_list;
	ShellPrintfCbk cbfunc;
	S32 is_used;
}ShellPrintfCbkListCb;


S32 ks_shell_init(char* project_name);
S32 ks_shell_uart_enable(S32 uart_id,S32 enable);
S32 ks_shell_str2uint( const char * str, uint32_t * pu32 );
S32 ks_shell_str2uint64(const char * str, uint64_t * pu64 );
S32 ks_shell_strhex2uint( const char * str, uint32_t * pu32 );

S32 ks_shell_printf(S32 uart_id,const char* format, ...);
S32 ks_shell_poll_printf(S32 uart_id,const char* format, ...);
S32 ks_shell_poll_printf_dump_hex(S32 uart_id,unsigned char * ptr, S32 len);
int ks_shell_printf_dump_hex(int uart_id,unsigned char * ptr, int len);
S32 ks_shell_input_handler(S32 uart_id, char byte);
S32 ks_shell_print_version(S32 uart_id);
S32 ks_shell_add_printf_callback( ShellPrintfCbk cbfunc);

void ks_shell_add_cmd( cmd_proc_t * cmd );
void ks_shell_add_cmds( cmd_proc_t * cmds, S32 len );
void ks_shell_remove_cmd( cmd_proc_t * cmd );
void ks_shell_remove_cmds( cmd_proc_t * cmds, S32 len );

char * ks_shell_get_project_name();


#ifdef __cplusplus
}
#endif


#endif

