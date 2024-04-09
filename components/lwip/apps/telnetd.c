
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#include "lwip/sockets.h"
#include "lwip/api.h"
#include "ks_shell.h"
#include "ks_printf.h"
#include "ks_taskdef.h"

#define  STACK_SIZE_LWIP_TLELNET_TASK          1024
static S32 thread_stack_lwip_telnetd_task[STACK_SIZE_LWIP_TLELNET_TASK];
static OSHandle flag_handle_lwip_telnetd_task;
static OSHandle thread_handle_lwip_telnetd_task;
static int init_flag_lwip_telentd;

//TelnetdContext  gTelnetdContext;

int client_fd = -1;    
int telnetd_std_out ;  

void reset_stdout(int fd){
    if(fd >= 0){
        close(fd);
    }
	telnetd_std_out = 0;
}


int Lwip_Telnetd_Is_Enable(){
	return telnetd_std_out;
}

void  Lwip_Telnetd_SocketPuts(  U32 uart_id, U8 *p_data, S32 len){
    if((client_fd >=0) && (get_socket_state(client_fd)>=0)) {
        write(client_fd, p_data, len);
    }
}

/*********************************************************************************************************
*                                      Lwip_TaskTelnetd  
*
* Description:    Telnetd 主任务 ，支持远程登录

* Arguments  : 
*
*            
*
* Returns    :  

* Note(s)    : 
*
*********************************************************************************************************/


void Lwip_TaskTelnetd(void *arg)
{  
    struct sockaddr_in addr = {AF_INET};
    socklen_t addrlen = sizeof(addr);	
    int telnet_server_fd = 0;
    int cmdok = 1;
    int cmdlen = 0;
    char cmdline[100] = "";

    int is_connected = 0 ;
  
    ks_os_printf(0,"task  telenet  is running \r\n");
    
    memset(&addr, 0, sizeof(addr));    
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(23);    

    telnet_server_fd = socket(AF_INET, SOCK_STREAM,0);
    if(telnet_server_fd < 0){
    	ks_os_printf(0,"[%s]creat socket error!\r\n", __func__);
    	return;
    }
    if(bind(telnet_server_fd, (struct sockaddr*)&addr, sizeof(addr))!=0){
    	ks_os_printf(0,"[%s]bind socket error!\r\n", __func__);
    	close(telnet_server_fd);
    	return;
    }
    listen(telnet_server_fd, 0);
	

	while(1){
		fd_set fdset;
		int cmd_length = 0;
		int max_fd = 0;
		FD_ZERO(&fdset);
		FD_SET(telnet_server_fd, &fdset);
		
		if(client_fd >= 0 && get_socket_state(client_fd)>=0){				
			FD_SET(client_fd, &fdset);
			if( cmdok==1 ){
				cmdok = 0;
				cmdlen = 0;
				Lwip_Telnetd_SocketPuts(0,(U8*)"telnet->",strlen("telnet->"));
			}
		}
		max_fd = (client_fd>telnet_server_fd)?client_fd:telnet_server_fd;
		if(select(max_fd+1, &fdset, NULL, NULL, NULL)>0){
			if(FD_ISSET(telnet_server_fd, &fdset)){
				int temp_fd = accept(telnet_server_fd, (struct sockaddr*)&addr, &addrlen);
				if(temp_fd >= 0){
					reset_stdout(client_fd);
					client_fd = temp_fd;
					telnetd_std_out = 1;
					ks_shell_printf(0,"connected from:%s\r\n", inet_ntoa(addr.sin_addr));
                                        is_connected = 1;
					ks_shell_printf(0,"Welcom telneted Server...\r\n");
					cmdok = 1;
				}
			}else if(FD_ISSET(client_fd, &fdset)){
				char *p = NULL;
				int readlen = 0;
				/*if(get_socket_state(client_fd)<0){
					reset_stdout();
                                        client_fd = -1;
					continue;
				}
				*/
				if(is_connected == 0){
	                client_fd = -1;   
	                continue;
	            }
				if(sizeof(cmdline)-cmdlen > 1){
					p = cmdline+cmdlen;
					readlen = read(client_fd, p, sizeof(cmdline)-cmdlen);
					if(readlen <= 0){
						reset_stdout(client_fd);
                                                is_connected = 0;
						client_fd = -1;
						continue;
					}
					p = strchr(cmdline, '\n');
					if(p!=NULL){
						*p = 0;
						cmdok = 1;
					}			
					p = strchr(cmdline, '\r');
					if(p!=NULL){
						*p = 0;
						cmdok = 1;
					}
					cmdlen += readlen;
				}else{
					cmdline[sizeof(cmdline)-1] = 0;
					cmdok = 1;
				}
				if(cmdok==1){
					if(strlen(cmdline)==0){
						continue;
					}
					
					ks_os_printf(0,"%s \r\n",cmdline);
#if 0
					for(int i = 0;i++;i<(int)strlen(cmdline)){
						ks_shell_input_handler(0,cmdline[i]);
					}
					ks_shell_input_handler(0,'\n');
					//exec_cmd(cmdline,strlen(cmdline));
#endif
				}
			}else{
				ks_shell_printf(0,"select out...\r\n");
			}
		}
	}


    
}


int quit(cmd_proc_t* ctx,int argc,  char **argv){
    if( (client_fd>=0) && (Lwip_Telnetd_Is_Enable())){
	    Lwip_Telnetd_SocketPuts(0,(U8*)"bye~\r\n",strlen("bye~\r\n"));
        ks_os_printf(0,"bye~\r\n");
        reset_stdout(client_fd);
        client_fd = -1;
    }
    return 0;
}

static cmd_proc_t telentd_cmds[] = {
    {.cmd = "quit", .fn = quit,  .help = "quit telentd"},

};

void ks_lwip_telentd_Init()
{
	if(init_flag_lwip_telentd == 0){


		ks_os_thread_create(&thread_handle_lwip_telnetd_task,			
								 "lwip_telnetd_task",					
								 Lwip_TaskTelnetd,						
								 0, 							
								 _THREAD_PRI_LWIP_SERVER,								
								 thread_stack_lwip_telnetd_task,			
								 sizeof(thread_stack_lwip_telnetd_task),	
								 0,
								 1
								 );
		 
		ks_shell_add_cmds(telentd_cmds, sizeof(telentd_cmds) / sizeof(cmd_proc_t));
		
		ks_shell_add_printf_callback(Lwip_Telnetd_SocketPuts);
		init_flag_lwip_telentd = 1;
	}
}
