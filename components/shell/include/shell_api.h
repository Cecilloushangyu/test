

#ifndef __SHELL_API_H__
#define __SHELL_API_H__
#include <stdint.h>

#include "dlist.h"
#include "ks_shell.h"



#define LINE_LEN 256
#define HISTORY_LEN 8




typedef struct shell_input_ctx {
	char buffer[LINE_LEN];
	int idx;
	int esc_level;
	int	history_next;
	int	history_cursor;
	char cmd_history_buffer[LINE_LEN * HISTORY_LEN];
	int uart_id;
	char cmd_str[LINE_LEN];
	char *argv[10];
	int argc;
	cmd_proc_t *cmd_proc;
	//int (*fn)(int argc, const char **argv );
}shell_input_ctx;




#endif 
