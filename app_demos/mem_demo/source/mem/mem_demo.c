
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  mem.c
 *
 * @brief  内存pool heap 分配、 任务之间消息通信演示实现
 */
#include <string.h>
#include <stdlib.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "mem_demo.h"
#include "ks_shell.h"
#include "ks_mem.h"




#define MSG_CLIENT1_SERVER_MSG_REQ   0x01
#define MSG_SERVER_CLIENT1_MSG_RSP   0x02

#define MSG_CLIENT2_SERVER_MSG_REQ   0x03
#define MSG_SERVER_CLIENT2_MSG_RSP   0x04

#define MSG_CLIENT1_TIMER_MSG_IND    0x05
#define MSG_CLIENT2_TIMER_MSG_IND    0x06
#define MSG_SERVER_CLIENT_MSG_IND    0x07


#define  STACK_SIZE_MEM_TASK          512

static S32 thread_stack_client1_task[STACK_SIZE_MEM_TASK];
static OSHandle thread_handle_client1_task;

static S32 thread_stack_client2_task[STACK_SIZE_MEM_TASK];
static OSHandle thread_handle_client2_task;

static S32 thread_stack_server_task[STACK_SIZE_MEM_TASK];
static OSHandle thread_handle_server_task;

static U32 g_client1_app_rx_queue[16];
static U32 g_client2_app_rx_queue[16];
static U32 g_server_app_rx_queue[16];

static OSHandle g_client1_app_mbx_handle;
static OSHandle g_client2_app_mbx_handle;
static OSHandle g_server_app_mbx_handle;

static OSHandle timer_client1 ;
static OSHandle timer_client2 ;




int msgtest(cmd_proc_t* ctx,int argc,  char **argv )
{
	int enable,client;

	if (argc >= 2 )
	{
	   client = atoi(argv[1]);
	   enable = atoi(argv[2]);

	   if(client == 0){
	   		if(enable)
		   	ks_os_timer_coarse_activate(timer_client1);
			else
			ks_os_timer_coarse_deactivate(timer_client1);
			
		   	ks_shell_printf(ctx->uart_id,"client 1 msgtest: %d \r\n",enable);
	   }
	   else if(client == 1)
	   {
		   if(enable)
		   ks_os_timer_coarse_activate(timer_client2);
		   else
		   ks_os_timer_coarse_deactivate(timer_client2);
		   
		   ks_shell_printf(ctx->uart_id,"client 2 msgtest: %d \r\n",enable);

	   }
	   else if(client > 1)
	   {
		   if(enable){
			   	ks_os_timer_coarse_activate(timer_client1);
				ks_os_timer_coarse_activate(timer_client2);

		   }else{
		   		ks_os_timer_coarse_deactivate(timer_client1);
			    ks_os_timer_coarse_deactivate(timer_client2);
		   }

		   ks_shell_printf(ctx->uart_id,"client1 client2 msgtest: %d \r\n",enable);

	   }
	}
	else
	{
		ks_shell_printf(ctx->uart_id,"msgtest <client> <enable/disable> \r\n");
	}

	return 0;

}



static cmd_proc_t mem_demo_cmds[] = {
	{.cmd = "msgtest", .fn = msgtest, .help = "msgtest <client> <enable/disable>"},
};



static void TimerClient1CallBack(void* arg)
{

    MessageBuffer * pinfo= ks_mem_pool_malloc(sizeof(MessageBuffer));
    if(pinfo==NULL){
        return ;
    }
    pinfo->msghead.msg_id = MSG_CLIENT1_TIMER_MSG_IND;

	ks_os_mbx_post(g_client1_app_mbx_handle,&pinfo);

}


static void Client1SendMsgToServer()
{
	static int count;
	char* str = "send by client1 !";
    MessageBuffer * pinfo= ks_mem_pool_malloc(sizeof(MessageBuffer)+strlen(str));
    if(pinfo==NULL){
        return ;
    }
    pinfo->msghead.msg_id = MSG_CLIENT1_SERVER_MSG_REQ;
	pinfo->msghead.param =  count++;
	strcpy((char*)pinfo->buffer,str);

	ks_os_mbx_post(g_server_app_mbx_handle,&pinfo);

}

void Client1AppTask(void *p_arg)
{
	ks_os_thread_vfp_enable();


	MessageHead * pmsg;

	ks_os_timer_coarse_create(&timer_client1, "timer_client1", TimerClient1CallBack, NULL, 100, 1000, 0);

	ks_shell_printf(0,"Client1AppTask 	start \r\n");

	while(1)
    {
		ks_os_mbx_pend(g_client1_app_mbx_handle, (void **)&pmsg, KS_OS_WAIT_FOREVER);
        switch(pmsg->msg_id){
			
            case MSG_SERVER_CLIENT1_MSG_RSP:
			ks_shell_printf(0,"Client1AppTask recieve MSG_SERVER_CLIENT1_MSG_RSP  %d \r\n",	pmsg->param);
            break;
			
			case MSG_CLIENT1_TIMER_MSG_IND:
			Client1SendMsgToServer();
			break;

		 	default:
            break;
        } 

		ks_mem_pool_free(pmsg);

    }
}

static void TimerClient2CallBack(void* arg)
{
    MessageBuffer * pinfo= ks_mem_pool_malloc(sizeof(MessageBuffer));
    if(pinfo==NULL){
        return ;
    }
    pinfo->msghead.msg_id = MSG_CLIENT2_TIMER_MSG_IND;

	ks_os_mbx_post(g_client2_app_mbx_handle,&pinfo);


}


static void Client2SendMsgToServer()
{
	static int count;
	char* str = "send by client2 !";
    MessageBuffer * pinfo= ks_mem_pool_malloc(sizeof(MessageBuffer)+strlen(str));
    if(pinfo==NULL){
        return ;
    }
    pinfo->msghead.msg_id = MSG_CLIENT2_SERVER_MSG_REQ;
	pinfo->msghead.param =  count++;
	strcpy((char*)pinfo->buffer,str);

	ks_os_mbx_post(g_server_app_mbx_handle,&pinfo);


}

void Client2AppTask(void *p_arg)
{
	ks_os_thread_vfp_enable();

	MessageHead * pmsg;

	ks_os_timer_coarse_create(&timer_client2, "timer_client2", TimerClient2CallBack, NULL, 1000, 1000, 0);

	ks_shell_printf(0,"Client2AppTask 	start \r\n");

	while(1)
    {
		ks_os_mbx_pend(g_client2_app_mbx_handle, (void **)&pmsg, KS_OS_WAIT_FOREVER);
        switch(pmsg->msg_id){
			
            case MSG_SERVER_CLIENT2_MSG_RSP:
			ks_shell_printf(0,"Client2AppTask recieve MSG_SERVER_CLIENT2_MSG_RSP  %d \r\n",	pmsg->param);
            break;
			
			case MSG_CLIENT2_TIMER_MSG_IND:
			Client2SendMsgToServer();
			break;

		 	default:
            break;
        } 
		ks_mem_pool_free(pmsg);
    }
}

void ServerAppTask(void *p_arg)
{
	ks_os_thread_vfp_enable();

	MessageBuffer * pmsg;
	ks_shell_printf(0,"ServerAppTask 	start \r\n");
	MessageHead * pinfo;


	while(1)
    {
		ks_os_mbx_pend(g_server_app_mbx_handle, (void **)&pmsg, KS_OS_WAIT_FOREVER);

        switch(pmsg->msghead.msg_id){
			
            case MSG_CLIENT1_SERVER_MSG_REQ:
			ks_shell_printf(0,"ServerAppTask recieve  MSG_CLIENT1_SERVER_MSG_REQ  %d  %s\r\n",	pmsg->msghead.param,pmsg->buffer);
			pinfo= ks_mem_pool_malloc(sizeof(MessageHead));
			if(pinfo==NULL){
				break ;
			}
			pinfo->msg_id = MSG_SERVER_CLIENT1_MSG_RSP;
			pinfo->param =pmsg->msghead.param;
			ks_os_mbx_post(g_client1_app_mbx_handle,&pinfo);

			break;
			
            case MSG_CLIENT2_SERVER_MSG_REQ:
			ks_shell_printf(0,"ServerAppTask recieve  MSG_CLIENT2_SERVER_MSG_REQ  %d  %s\r\n",	pmsg->msghead.param,pmsg->buffer);
			pinfo= ks_mem_pool_malloc(sizeof(MessageHead));
			if(pinfo==NULL){
				break ;
			}
			pinfo->msg_id = MSG_SERVER_CLIENT2_MSG_RSP;
			pinfo->param =pmsg->msghead.param;
			ks_os_mbx_post(g_client2_app_mbx_handle,&pinfo);

			break;

		 	default:
            break;
        }

		ks_mem_pool_free(pmsg);
    }
}


void MemDemoInit(void)
{

	
	ks_shell_add_cmds(mem_demo_cmds, sizeof(mem_demo_cmds) / sizeof(cmd_proc_t));

	ks_os_mbx_create(&g_client1_app_mbx_handle,"client1_app mbox", g_client1_app_rx_queue, 16);
	ks_os_mbx_create(&g_client2_app_mbx_handle,"client2_app mbox", g_client2_app_rx_queue, 16);
	ks_os_mbx_create(&g_server_app_mbx_handle,"server_app mbox", g_server_app_rx_queue, 16);

	ks_os_thread_create(&thread_handle_server_task,			
						 "server_app_task",					
						 ServerAppTask,						
						 0,								
						 12,								
						 thread_stack_server_task,			
						 sizeof(thread_stack_server_task),	
						 0,
						 1
						 );


	ks_os_thread_create(&thread_handle_client1_task,			 
						  "client1_app_task",					 
						  Client1AppTask,						 
						  0,							 
						  13,								 
						  thread_stack_client1_task,		 
						  sizeof(thread_stack_client1_task), 
						  0,
						  1
						  );

	ks_os_thread_create(&thread_handle_client2_task,			 
						  "client2_app_task",					 
						  Client2AppTask,						 
						  0,							 
						  14,								 
						  thread_stack_client2_task,		 
						  sizeof(thread_stack_client2_task), 
						  0,
						  1
						  );

	
}




