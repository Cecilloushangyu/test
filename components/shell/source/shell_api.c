
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include "shell_api.h"
#include "shell_cmd.h"
#include "ks_taskdef.h"
#include "ks_printf.h"
#include "ks_os.h"
#include "ks_uart.h"





#define UART_NUM  4



static char g_shell_input_buffer[LINE_LEN];

static shell_input_ctx g_shell_input_ctx[UART_NUM];

static shell_input_ctx g_shell_input_ctx_post;

static const char *g_shell_prompt_str = "";

static int g_shell_init;

static char g_shell_output_buffer[8 * 1024] __attribute__((aligned(8)));

static cmd_proc_t *g_shell_cmds = NULL;


#define  STACK_SIZE_SHELL          1024
static S32 thread_stack_shell[STACK_SIZE_SHELL];

static OSHandle thread_handle_shell;



#define	SHELL_RX_QUEUE_NUM		16
static OSHandle g_shell_mbx_handle;


U32 g_shell_rx_queue[SHELL_RX_QUEUE_NUM];

static char g_shell_project_name[64];

struct list_node  g_shell_printf_callback_list;	

ShellPrintfCbkListCb s_shellprintfcb[4];

void ks_shell_add_cmd( cmd_proc_t * cmd )
{
	cmd->next = g_shell_cmds;
	g_shell_cmds = cmd;
}
void ks_shell_add_cmds( cmd_proc_t * cmds, int len )
{
	int i;

	for (i = 0; i < len; i++ )
	{
		ks_shell_add_cmd( cmds++ );
	}
}

void ks_shell_remove_cmd( cmd_proc_t * cmd )
{

	cmd_proc_t *p = g_shell_cmds;
	cmd_proc_t *front=g_shell_cmds;

	if (NULL == cmd)
	{
		return ;
	}
	if (g_shell_cmds == NULL)
	{
		return ;
	}
	
	
	while (p)
	{
		if (0 == strcmp(cmd->cmd, p->cmd))
		{	
			if(front == g_shell_cmds )
			g_shell_cmds = p->next;
			else
			front->next = p->next;
			return ;
		}
		
		front =p;
		p = p->next;
	
	}
}

void ks_shell_remove_cmds( cmd_proc_t * cmds, int len )
{
	int i;

	for (i = 0; i < len; i++ )
	{
		ks_shell_remove_cmd( cmds++ );
	}
}

char * ks_shell_get_project_name(){
	return g_shell_project_name;
}


cmd_proc_t* get_shell_cmds_head( ){
	return	g_shell_cmds;
}

static inline uint32_t ptrnext(uint32_t ptr)
{
    return (ptr + 1) % HISTORY_LEN;
}

static inline uint32_t ptrprev(uint32_t ptr)
{
    return (ptr - 1) % HISTORY_LEN;
}

static inline char *shell_history_line(shell_input_ctx* input,uint32_t line)
{
    return input->cmd_history_buffer + line * LINE_LEN;
}


static void shell_add_history(shell_input_ctx* input,const char *line)
{
    // reject some stuff
    uint32_t last ;
    if (line[0] == 0)
    {
        goto add_h_finish;
    }

    last= ptrprev(input->history_next);
    if (strcmp(line, shell_history_line(input,last)) == 0)
        goto add_h_finish;

    strncpy(shell_history_line(input,input->history_next), line, LINE_LEN);
    input->history_next = ptrnext(input->history_next);

add_h_finish:
    input->history_cursor = ptrprev(input->history_next);
}

static const char *shell_next_history(shell_input_ctx* input,int *cursor)
{
    int i = ptrnext(*cursor);

    if (i == input->history_next)
        return ""; // can't let the cursor hit the head

    *cursor = i;
    return shell_history_line(input,i);
}


static const char *shell_prev_history(shell_input_ctx* input,int *cursor)
{
    int i;
    const char *str = shell_history_line(input,*cursor);

    /* if we are already at head, stop here */
    if (*cursor == input->history_next)
        return str;

    /* back up one */
    i = ptrprev(*cursor);

    /* if the next one is gonna be null */
    if (shell_history_line(input,i)[0] == '\0')
        return str;

    /* update the cursor */
    *cursor = i;
    return str;
}

unsigned long long int ks_shell_strtoull(const char *ptr, char **end, int base)
{
	unsigned long long ret = 0;
	if (base > 36)
		goto out;
	while (*ptr)
	{
		int digit;
		if (*ptr >= '0' && *ptr <= '9' && *ptr < '0' + base)
			digit = *ptr - '0';
		else if (*ptr >= 'A' && *ptr < 'A' + base - 10)
			digit = *ptr - 'A' + 10;
		else if (*ptr >= 'a' && *ptr < 'a' + base - 10)
			digit = *ptr - 'a' + 10;
		else
			break;
		ret *= base;
		ret += digit;
		ptr++;
	}
out:
	if (end)
		*end = (char *)ptr;
	return ret;
}

int ks_shell_strhex2uint( const char * str, uint32_t * pu32 )
{
	char * pend;

	uint64_t  temp;

	temp = ks_shell_strtoull( str, &pend, 16 );
	if ( pend[0] != 0 )
	{
		return 1;
	}

	
	if ( temp > 0xffffffffull )
	{
		return 2;
	}

	*pu32 = (uint32_t)temp;

	return 0;


}

int ks_shell_str2uint( const char * str, uint32_t * pu32 )
{
	char * pend;
	uint64_t  temp;

	if ( str[0] == 0x30 )
	{
		if ( str[1] == 0 )
		{
			*pu32 = 0;
			return 0;
		}
		
		if ( (str[1] == 0x58) || (str[1] == 0x78) )
		{
			
			temp = ks_shell_strtoull( str, &pend, 16 );
			if ( pend[0] != 0 )
			{
				return 1;
			}

			
			if ( temp > 0xffffffffull )
			{
				return 2;
			}
		
			*pu32 = (uint32_t)temp;
			return 0;
		}
		else
		{
			return 3;
		}
	}

	
	if ( (str[0] >= 0x31) && (str[0] <= 0x39) )
	{
		
		temp = ks_shell_strtoull( str, &pend, 10 );
		if ( pend[0] != 0 )
		{
			return 4;
		}

		if ( temp > 0xffffffffull )
		{
			return 5;
		}
		
		*pu32 = (uint32_t)temp;		
		return 0;
	}

	
	return 6;
}

int ks_shell_str2uint64(const char * str, uint64_t * pu64 )
{
	char * pend;
	uint64_t  temp;

	if ( str[0] == 0x30 )
	{
		if ( str[1] == 0 )
		{
			*pu64 = 0;
			return 0;
		}
		
		if ( (str[1] == 0x58) || (str[1] == 0x78) )
		{
			
			temp = ks_shell_strtoull( str, &pend, 16 );
			if ( pend[0] != 0 )
			{
				return 1;
			}

			
			if ( temp > 0xffffffffull )
			{
				return 2;
			}
		
			*pu64 = temp;
			return 0;
		}
		else
		{
			return 3;
		}
	}

	
	if ( (str[0] >= 0x31) && (str[0] <= 0x39) )
	{
		
		temp = ks_shell_strtoull( str, &pend, 10 );
		if ( pend[0] != 0 )
		{
			return 4;
		}

		if ( temp > 0xffffffffull )
		{
			return 5;
		}
		
		*pu64 = temp;		
		return 0;
	}

	
	return 6;
}


int ks_shell_printf(int uart_id,const char* format, ...)
{
	va_list va;
	int ret;
	int wlen;

	va_start(va, format);
	wlen = vsnprintf_(g_shell_output_buffer, sizeof(g_shell_output_buffer), format, va);
	va_end(va);
	
	ShellPrintfCbkListCb * pcallback;

	list_for_every_entry( &g_shell_printf_callback_list, pcallback, ShellPrintfCbkListCb, cb_list )
	{	
		if(pcallback->is_used&&pcallback->cbfunc!=NULL){
			pcallback->cbfunc(uart_id, (U8*)g_shell_output_buffer,wlen);
		}						
	}

	ks_driver_uart_send_buffer(uart_id, (U8*)g_shell_output_buffer,wlen);

	return wlen;

}

int ks_shell_poll_printf(int uart_id,const char* format, ...){

	va_list va;
	int ret;
	int wlen;

	va_start(va, format);
	wlen = ks_vsnprintf(g_shell_output_buffer, sizeof(g_shell_output_buffer), format, va);
	va_end(va);

	ks_driver_uart_poll_send_buffer(uart_id, g_shell_output_buffer,wlen);


	return wlen;

}

int ks_shell_poll_printf_dump_hex(int uart_id,unsigned char* ptr, int len){

	int  i;
	int  nn;
	int  len2 = len;


	nn = 0;
	while ( (len2 - nn) >= 16 )
	{
		ks_shell_poll_printf(uart_id, "%08lx: ", (uintptr_t)(ptr + nn) );
		
		for ( i=0; i<16; i++ )
		{
			ks_shell_poll_printf(uart_id,  "%02x ", ptr[nn + i] );
			//ks_shell_printf(uart_id,  "0x%02x,", ptr[nn + i] );
		}

		nn += 16;
		ks_shell_poll_printf(uart_id, "\r\n");
		
	}

	if ( len2 > nn )
	{
		ks_shell_poll_printf( uart_id, "%08lx: ", (uintptr_t)(ptr + nn) );
		
		for ( i = 0; i < (len2-nn); i++ )
		{
			ks_shell_poll_printf(uart_id, "%02x ", ptr[nn + i]);
			//ks_shell_printf(uart_id,  "0x%02x,", ptr[nn + i] );
		}

		
		for ( ; i < 16; i++ )
		{
			ks_shell_poll_printf(uart_id,  "   " );
		}
		

		ks_shell_poll_printf(uart_id, "\r\n");
		
	}

	return 0 ;
}

int ks_shell_printf_dump_hex(int uart_id,unsigned char * ptr, int len){

	int  i;
	int  nn;
	int  len2 = len;


	nn = 0;
	while ( (len2 - nn) >= 16 )
	{
		ks_shell_printf(uart_id, "%08lx: ", (uintptr_t)(ptr + nn) );
		
		for ( i=0; i<16; i++ )
		{
			ks_shell_printf(uart_id,  "%02x ", ptr[nn + i] );
			//ks_shell_printf(uart_id,  "0x%02x,", ptr[nn + i] );
		}

		nn += 16;
		ks_shell_printf(uart_id, "\r\n");
		
	}

	if ( len2 > nn )
	{
		ks_shell_printf( uart_id, "%08lx: ", (uintptr_t)(ptr + nn) );
		
		for ( i = 0; i < (len2-nn); i++ )
		{
			ks_shell_printf(uart_id, "%02x ", ptr[nn + i]);
			//ks_shell_printf(uart_id,  "0x%02x,", ptr[nn + i] );
		}

		
		for ( ; i < 16; i++ )
		{
			ks_shell_printf(uart_id,  "   " );
		}
		

		ks_shell_printf(uart_id, "\r\n");
		
	}

	return 0 ;
}


static cmd_proc_t* shell_find_cmd_proc( char *cmd){


	cmd_proc_t *p = g_shell_cmds;

	if (NULL == cmd)
	{
		return NULL;
	}
	if (g_shell_cmds == NULL)
	{
		return NULL;
	}

	while (p)
	{
		if (0 == strcmp(cmd, p->cmd))
		{
			if (p->fn != NULL)
			{
				return p;
			}
		}

		p = p->next;
	}

	return 0 ;

}

//return 1 , find shell cmd , hande it, inform other parse module ;  
int  ks_shell_input_handler(int uart_id, char byte)
{
	uint8_t temp;
	int ret = 0;

	char *p;
	int i;

	shell_input_ctx* input_ctx;

	if (uart_id == -1)
	{
	   for( i= 0;i< UART_NUM;i++){
		   input_ctx = &g_shell_input_ctx[i];
		   input_ctx->idx = 0;
	   }
	   return 0;
	}

	input_ctx = &g_shell_input_ctx[uart_id];
	input_ctx->uart_id = uart_id;

	temp = byte;

	if ((temp != 0x0A) && (temp != 0x0D))
	{
	   if (input_ctx->esc_level == 0)
	   {
		   if (temp == 0x7f || temp == '\b')
		   {
			   if (input_ctx->idx > 0)
			   {
				   ks_shell_printf(uart_id,"\b \b");
				   input_ctx->idx--;
			   }
		   }
		   else if (temp == 0x1b)
		   {
			   input_ctx->esc_level = 1;
		   }
		   else
		   {
			   input_ctx->buffer[input_ctx->idx] = (char)temp;
			   input_ctx->idx += 1;
			   // 超过最大长度，不是shell命令，直接返回　
			   if (input_ctx->idx >= LINE_LEN)
			   {
				   input_ctx->idx = 0;
				   // ks_shell_printf(uart_id,"\nstring is too long, try again\n");
				   // ks_shell_printf(uart_id,">> ");
				   return 0;
			   }

			   ks_shell_printf(uart_id,"%c", temp);
		   }
	   }
	   else if (input_ctx->esc_level == 1)
	   {
		   if (temp == '[')
			   input_ctx->esc_level = 2;
		   else
			   input_ctx->esc_level = 0;
	   }
	   else
	   { 
		   int i;
		   //up arrow
		   if (temp == 65)
		   {
			   //check g_shell_cmd_history_buffer
			   strncpy(input_ctx->buffer, shell_prev_history(input_ctx,&input_ctx->history_cursor), LINE_LEN);
			   //clear current cmd
			   for (i = 0; i < input_ctx->idx; i++)
				    ks_shell_printf(uart_id,"\b \b");

			   input_ctx->idx = strlen(input_ctx->buffer);
			    ks_shell_printf(uart_id,"%s", input_ctx->buffer);
		   }
		   else if (temp == 66)
		   { //down arrow
			   strncpy(input_ctx->buffer, shell_next_history(input_ctx,&input_ctx->history_cursor), LINE_LEN);
			   for (i = 0; i < input_ctx->idx; i++)
				    ks_shell_printf(uart_id,"\b \b");
			   input_ctx->idx = strlen(input_ctx->buffer);
			    ks_shell_printf(uart_id,"%s", input_ctx->buffer);
		   }
		   input_ctx->esc_level = 0;
	   }
	   return 0;
	}

	ks_shell_printf(uart_id,"\r\n");

	input_ctx->buffer[input_ctx->idx] = '\0';
	input_ctx->argc = 0;
	p = input_ctx->buffer;

	shell_add_history(input_ctx,input_ctx->buffer);
	strcpy(input_ctx->cmd_str,input_ctx->buffer);
	input_ctx->cmd_str[input_ctx->idx] = '\0';
   	while (1)
	{
		if (*p == '\0')
			break;
		else if (*p == ' ' || *p == '\t')
			*p++ = '\0';
		else
		{
			input_ctx->argv[input_ctx->argc++] = p;
			while (1)
			{
				if ((*p == '\0') || (*p == ' ') || (*p == '\t'))
					break;
				p++;
			}
		}
	}
		
   input_ctx->cmd_proc = shell_find_cmd_proc(input_ctx->argv[0]);
   if(input_ctx->cmd_proc!=NULL){
	   if(g_shell_mbx_handle!=0){
			memcpy(&g_shell_input_ctx_post,input_ctx,sizeof(shell_input_ctx));
			void* temp =(void* )&g_shell_input_ctx_post;
	   		ks_os_mbx_post(g_shell_mbx_handle,&temp);
	   }
   }
   input_ctx->idx = 0;
   ks_shell_printf(uart_id,"%s>>", g_shell_prompt_str);
   return ret;

}


int ks_shell_add_printf_callback( ShellPrintfCbk cb_func){

	ShellPrintfCbkListCb* precvcb = NULL;

	int count = sizeof(s_shellprintfcb)/sizeof(s_shellprintfcb[0]);
	for(int i = 0;i<count;i++)
	{
		if(s_shellprintfcb[i].is_used==0)
		{
			precvcb = &s_shellprintfcb[i];
			precvcb->is_used=1;
			break;
		}
	}

	if(precvcb!=NULL){
		precvcb->cbfunc=cb_func;
		list_add_tail( &(g_shell_printf_callback_list), &(precvcb->cb_list) );
		return 0 ;
	}else{
		return -1;
	}

}



static int shell_process_cmd(shell_input_ctx* input_ctx)
{
	int ret;
	cmd_proc_t *p ;
	int uart_id = input_ctx->uart_id;
	int argc= input_ctx->argc;
	char **argv= input_ctx->argv;;
	//ks_shell_printf(uart_id," %d  %s \r\n",uart_id,input_ctx->buffer);
	p= input_ctx->cmd_proc;
	p->uart_id = uart_id;
	ret = p->fn(p,argc, argv);
	if (ret)
	{
	    if (ret == CMD_ERR_PARAMS_FORMAT)
	    {
	        ks_shell_printf(uart_id,"params format error\r\n");
	    }
	    else if (ret == CMD_ERR_PARAMS_NOT_ENOUGH)
	    {
	        ks_shell_printf(uart_id,"number of params is incorrect\r\n");
	    }
	    else if (ret == CMD_ERR_PARAM_NOT_EXIST)
	    {
	        ks_shell_printf(uart_id,"cmd does not exist\r\n");
	    }
	    else if (ret == CMD_ERR_DEVICE_OP_FAILED)
	    {
	        ks_shell_printf(uart_id,"device operation failed\r\n");
	        ret = 0;
	    }

	    if (ret && p->help)
	        ks_shell_printf(uart_id,"Usage:\n\t%s\r\n", p->help);

	    return 0;
	}


	return 0;
}




void shell_task(void *p_arg)
{

	void* input;
	shell_input_ctx* input_ctx;
	while(1)
	{
		ks_os_mbx_pend(g_shell_mbx_handle, (void **)&input, KS_OS_WAIT_FOREVER);
		
		input_ctx= (shell_input_ctx*)input;
		shell_process_cmd(input_ctx);
	}
}


int ks_shell_uart_enable(int uart_id,int enable)
{
	if(uart_id >=0 && uart_id < UART_NUM){
		if(enable)
		ks_driver_uart_register_protocol(uart_id,ks_shell_input_handler);
		else
		ks_driver_uart_deregister_protocol(uart_id,ks_shell_input_handler);

		return 0;
	}

	return -1;
}



//初始化shell模块，　提供shell api ，方便其他模块添加交互调试命令
int ks_shell_init(char* project_name)
{

	if(g_shell_init == 0){

		uint32_t len = strlen(project_name);
		memcpy(g_shell_project_name,project_name, len>sizeof(g_shell_project_name)? sizeof(g_shell_project_name):len);

		list_initialize(&g_shell_printf_callback_list);

		//add system cmd 
		shell_cmd_init();

		ks_os_mbx_create(&g_shell_mbx_handle,"shell mbox", g_shell_rx_queue, SHELL_RX_QUEUE_NUM);

		ks_os_thread_create(&thread_handle_shell,		
					  "shell task",	 
					  shell_task, 				 
					  0,								 
					  22,			
					  thread_stack_shell,			
					  sizeof(thread_stack_shell),
					  0,
					  1
					  );
		
		g_shell_init = 1;
	}
    return 0;
}







