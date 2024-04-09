
#include <stdint.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>


#include <sys/types.h>          /* See NOTES */
#include <lwip/sockets.h>
#include <string.h>


#include <unistd.h>
#include <stdio.h>

#include "ks_include.h"
#include "ks_shell.h"
#include "ks_printf.h"

#include "ipaddr_utils.h" 



#define SERVER_UDP_PORT 8088
#define SERVER_TCP_PORT 8089

#define BACKLOG     10



#define BUFFER_LEN     1500


static char  udp_dst_ipaddr[16];



static char  tcp_dst_ipaddr[16];


#define TRACE_ERR kprintf


static OSHandle timer_socket ;


#define  STACK_SIZE_TCP_SERVER_TASK          1024
static S32 thread_stack_tcp_server_task[STACK_SIZE_TCP_SERVER_TASK];

static OSHandle thread_handle_tcp_server_task;



#define  STACK_SIZE_UDP_SERVER_TASK          1024
static S32 thread_stack_udp_server_task[STACK_SIZE_UDP_SERVER_TASK];

static OSHandle thread_handle_udp_server_task;



int SocketDemo_ClientSendUdp(char * dst_ip )
{
    int sendcount =0;      
	int iSocketClient =-1;
	struct sockaddr_in tSocketServerAddr;
    sendcount ++;
    int iRet;
    char ucSendBuf[BUFFER_LEN];
	char ucRecvBuf[BUFFER_LEN];
    int iSendLen;
    int iAddrLen;
    char * Test_string=" This Is a  Lwip Demo  Client  Send   Udp  String  "; 
    
	iSocketClient = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    ks_os_printf(0,"socket %d   \r\n",iSocketClient);
    if(iSocketClient == -1){
		return -1;
	}

    memset(&tSocketServerAddr, 0, sizeof(tSocketServerAddr)); 
    tSocketServerAddr.sin_family = AF_INET;
    tSocketServerAddr.sin_port = htons(SERVER_UDP_PORT);  /* host to net, short */
    memset(tSocketServerAddr.sin_zero, 0, 8);

	if( tSocketServerAddr.sin_addr.s_addr!=inet_addr(dst_ip) ){
		tSocketServerAddr.sin_addr.s_addr = inet_addr(dst_ip);
	}   
	
    strcpy(ucSendBuf,Test_string);
    ks_os_printf(0,"SocketDemo_ClientSendUdp dst_ip %s port %d !\r\n" ,dst_ip,ntohs(tSocketServerAddr.sin_port));
	ks_os_printf(0,"len: %d  %s  !\r\n" , strlen(ucSendBuf),ucSendBuf);


    iAddrLen = sizeof(struct sockaddr);
    iSendLen = sendto(iSocketClient, ucSendBuf, strlen(ucSendBuf), 0,(const struct sockaddr *)&tSocketServerAddr, iAddrLen);

	//recvfrom(iSocketClient, ucRecvBuf, BUFFER_LEN, 0, &tSocketServerAddr, &iAddrLen);
	//ks_shell_printf(ctx->uart_id,"recv from server : %s\r\n", ucRecvBuf);

	close(iSocketClient);

    return 0;

}




void SocketDemo_SeverRecieveUdp( ){

    int iSocketServer;
    int iSocketClient;
    struct sockaddr_in tSocketServerAddr;
    struct sockaddr tSocketClientAddr;
    int iRet;
    socklen_t iAddrLen;

    int iRecvLen;
    unsigned char ucRecvBuf[BUFFER_LEN];

    int iClientNum = -1;
    
    ks_os_printf(0,"SocketDemo_SeverRecieveUdpTest  !\r\n");
    
    // 创建服务器 socket 
    iSocketServer = socket(AF_INET, SOCK_DGRAM, 0);
    if (-1 == iSocketServer)
    {
    	ks_os_printf(0,"socket error!\r\n");
    	return ;
    }
    // 初始化 
    tSocketServerAddr.sin_family      = AF_INET;
    tSocketServerAddr.sin_port        = htons(SERVER_UDP_PORT);  /* host to net, short */
    tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
    memset(tSocketServerAddr.sin_zero, 0, 8);
    // bind 
    iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
    if (-1 == iRet)
    {
    	ks_os_printf(0,"bind error!\r\n");
    	return ;
    }


    while (1)
    {
    	iAddrLen = sizeof(struct sockaddr);
    	iRecvLen = recvfrom(iSocketServer, ucRecvBuf, BUFFER_LEN, 0, &tSocketClientAddr, &iAddrLen);
    	if (iRecvLen > 0)
    	{
    		ucRecvBuf[iRecvLen] = '\0';
    		struct sockaddr_in* sock_in = ( struct sockaddr_in*)&tSocketClientAddr;
    		ks_os_printf(0,"Recv Msg From %s  port %d  \r\n", inet_ntoa(sock_in->sin_addr), ntohs(sock_in->sin_port));
			ks_os_printf(0,"%s \r\n", ucRecvBuf);

			sendto(iSocketServer, ucRecvBuf, iRecvLen, 0, &tSocketClientAddr, iAddrLen);
       		 /* 清空缓冲区 */
        	memset(ucRecvBuf, 0, BUFFER_LEN);
    	}
    }

    close(iSocketServer);
	
    return ;


}

int SocketDemo_ClientSendTcp(char * dst_ip)
{
    static int iSocketClient =-1;
	static int iSocketConnect = -1;
    struct sockaddr_in tSocketServerAddr;
    char ucSendBuf[BUFFER_LEN];
	char ucRecvBuf[BUFFER_LEN];
    int iSendLen;

    char * Test_string=" This Is a  Lwip Demo  Client  Send   Tcp String  "; 
	memset(ucSendBuf, 0, BUFFER_LEN);   
	memset(ucRecvBuf, 0, BUFFER_LEN);  

	//has conneted server 
	if(iSocketConnect!=-1){
		goto op_send;
	}
    iSocketClient = socket(AF_INET, SOCK_STREAM, 0);
    if(iSocketClient == -1){
		return -1;
	}
	ks_os_printf(0,"1.socket: %d !\r\n" , iSocketClient);

    tSocketServerAddr.sin_family      = AF_INET;
    tSocketServerAddr.sin_port        = htons(SERVER_TCP_PORT);  /* host to net, short */
    tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
    if (0 == inet_aton(dst_ip, &tSocketServerAddr.sin_addr))
    {
        ks_os_printf(0,"invalid server_ip \r\n");
        return -1;
    }
    memset(tSocketServerAddr.sin_zero, 0, 8);
	
    int ret = connect(iSocketClient, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));	
    if (-1 == ret)
    {
        ks_os_printf(0,"connect error!\r\n");
		close(iSocketClient);
        return -1;
    }
	ks_os_printf(0,"2.connect: %d !\r\n" , iSocketClient);


	struct timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=200;
	setsockopt(iSocketClient,SOL_SOCKET,SO_RCVTIMEO,(const void*)&tv,sizeof( struct timeval));

op_send:
    strcpy(ucSendBuf,Test_string);
	iSocketConnect = iSocketClient;
    iSendLen = send(iSocketClient, ucSendBuf, strlen(ucSendBuf), 0);
    if (iSendLen <= 0)
    {
        TRACE_ERR("send error!  %d \r\n",iSendLen);
    }
	ks_os_printf(0,"3.send dst_ip %s port %d !\r\n" ,dst_ip,ntohs(tSocketServerAddr.sin_port));
	//recv(iSocketClient, ucRecvBuf, BUFFER_LEN, 0);
	//ks_shell_printf(ctx->uart_id,"4.recv from server : %s\r\n", ucRecvBuf);
	
	close(iSocketClient);
	ks_os_printf(0,"5.close\r\n");
	iSocketConnect = -1;
	
    return 0;
}




void SocketDemo_SeverRecieveTcp()
{
	int iSocketServer;
	int iSocketClient;
	struct sockaddr_in tSocketServerAddr;
	struct sockaddr_in tSocketClientAddr;
	int iRet;
	socklen_t iAddrLen;
    int optval = 1;

	int iRecvLen;
	char ucRecvBuf[BUFFER_LEN];
    char ucSendBuf[BUFFER_LEN];
    char * Test_string="service success recieve! "; 
    strcpy(ucSendBuf,Test_string);

	int iClientNum = -1;
    // 创建服务器 socket 
	iSocketServer = socket(AF_INET, SOCK_STREAM, 0);
	if (-1 == iSocketServer)
	{
		ks_os_printf(0,"socket error!\r\n");
		return ;
	}
    // 初始化 
	tSocketServerAddr.sin_family      = AF_INET;
	tSocketServerAddr.sin_port        = htons(SERVER_TCP_PORT);  /* host to net, short */
 	tSocketServerAddr.sin_addr.s_addr = INADDR_ANY;
	memset(tSocketServerAddr.sin_zero, 0, 8);


	// bind 
	iRet = bind(iSocketServer, (const struct sockaddr *)&tSocketServerAddr, sizeof(struct sockaddr));
	if (-1 == iRet)
	{
		ks_os_printf(0,"bind error!\r\n");
		return ;
	}
    // listen 
	iRet = listen(iSocketServer, BACKLOG);
	if (-1 == iRet)
	{
		ks_os_printf(0,"listen error!\r\n");
		return ;
	}
	
	struct timeval tv;
	tv.tv_sec=0;
	tv.tv_usec=200;
	setsockopt(iSocketServer,SOL_SOCKET,SO_RCVTIMEO,(const void*)&tv,sizeof( struct timeval));

    ks_os_printf(0,"SocketDemo_SeverRecieveTcp \r\n");
	while (1)
	{
		iAddrLen = sizeof(struct sockaddr);
        // accept 
        // 监听客户端请求，accept函数返回一个新的套接字，发送和接收都是用这个套接字 
		iSocketClient = accept(iSocketServer, (struct sockaddr *)&tSocketClientAddr, &iAddrLen);
		if (-1 == iSocketClient)
		{
			TRACE_ERR("accept err  %d\r\n", iSocketClient);
			close(iSocketServer);
			return ;
		}
#if 0
		setsockopt(iSocketClient,
				   IPPROTO_TCP, 	/* set option at TCP level */
				   TCP_NODELAY, 	/* name of option */
				   (void *) &optval,	/* the cast is historical cruft */
				   sizeof(int));	/* length of option value */
		
		
	   	struct timeval tv;
	   	tv.tv_sec=0;tv.tv_usec=200;
	   	setsockopt(iSocketClient,SOL_SOCKET,SO_RCVTIMEO,(const char*)&tv,sizeof( struct timeval));
#endif
		iClientNum++;
		ks_os_printf(0,"get connect from client %d : %s\r\n",  iClientNum, inet_ntoa(tSocketClientAddr.sin_addr));

		while(1)
	    {
	       // recv 接收客户端发过来的消息
	      iRecvLen = recv(iSocketClient, ucRecvBuf, BUFFER_LEN, 0);
	      
	      if (iRecvLen <= 0) 
	        break;

		  ucRecvBuf[iRecvLen] = '\0';
		  ks_os_printf(0,"recv Client %d: %s\r\n", iClientNum, ucRecvBuf);
	      write(iSocketClient,ucSendBuf,strlen(ucSendBuf));
	    }
		
	    if (iSocketClient >= 0) {
			ks_os_printf(0,"close %d: \r\n", iSocketClient);
			close(iSocketClient);
		}

	}
	
	close(iSocketServer);

}



  
int udpsend(cmd_proc_t* ctx, int argc,  char **argv  )
{
    int  iret;
	uint32_t type;

	uint32_t count;
    if ( argc != 3 )
    {
       goto usage;
    }

    if(ip_isvalued(argv[1]) == 1){
        ks_shell_printf(ctx->uart_id, "invalued <desip> \n", argv[1] );
        return -1;
    }

	
    iret = ks_shell_str2uint( argv[2], &count );
    if ( iret != 0 )
    {
	    goto usage;
    }
    memset(udp_dst_ipaddr,0,16);  
    strcpy(udp_dst_ipaddr,argv[1]);

	SocketDemo_ClientSendUdp(udp_dst_ipaddr);
    return 0;

usage:
    ks_shell_printf(ctx->uart_id, "usage: %s  <desip>  <count> \n", argv[0] );
    return 0;
    
}




int tcpsend( cmd_proc_t* ctx,int argc,  char **argv  )
{
    int  iret;
	uint32_t type;

	uint32_t count;
    if ( argc != 3 )
    {
       goto usage;
    }

    if(ip_isvalued(argv[1]) == 1){
        ks_shell_printf(ctx->uart_id, "invalued <desip> \n", argv[1] );
        return 0;
    }

	
    iret = ks_shell_str2uint( argv[2], &count );
    if ( iret != 0 )
    {
	    goto usage;
    }
	
    memset(tcp_dst_ipaddr,0,16);  
    strcpy(tcp_dst_ipaddr,argv[1]);

	SocketDemo_ClientSendTcp(tcp_dst_ipaddr);
	
    return 0;

usage:
    ks_shell_printf(ctx->uart_id, "usage: %s  <desip>  <count> \n", argv[0] );
    return 0;
    
}






static void timer_socket_cb(void* arg){

	static int seqid = 0;
	int ret ;
	int socket_type = (int )arg;
	if(socket_type == 0){
		SocketDemo_ClientSendUdp(udp_dst_ipaddr);
	}
	else{
		SocketDemo_ClientSendTcp(tcp_dst_ipaddr);

	}


}


static int socket_tick_cmd(cmd_proc_t* ctx,int argc,  char **argv)
{
    int iret;
	uint32_t enable;
	uint32_t socket_type;

	enable = 0;
	
	if (argc >= 2)
	{
		iret = ks_shell_str2uint(argv[1], &enable);
		if (iret != 0)
		{
			return CMD_ERR_PARAMS_FORMAT;
		}
		if(argc > 2){
		   iret = ks_shell_str2uint(argv[2], &socket_type);
		   if (iret != 0)
		   {
			   return CMD_ERR_PARAMS_FORMAT;
		   }
		}
	}
	else
	{
	  	return 0;
	}
	
	if(enable){
		if(timer_socket == 0){
			ks_os_timer_coarse_create(&timer_socket, "timer_socket", timer_socket_cb, (void*)socket_type, 100, 1000, 0);
		}


		ks_os_timer_coarse_activate(timer_socket);
	}else{
		if(timer_socket != 0)
		ks_os_timer_coarse_deactivate(timer_socket);
	}

	ks_shell_printf(ctx->uart_id,"sockettick : %d \r\n",enable);

    return 0;
}



static cmd_proc_t socket_demo_cmds[] = {
	{.cmd = "sockettick", .fn = socket_tick_cmd,  .help = "sockettick  <enable/disable> <socket_type>"},
	{.cmd = "udpsend", .fn = udpsend, .help = "udpsend"},
	{.cmd = "tcpsend", .fn = tcpsend, .help = "tcpsend"},

};


void SocketDemoInit(void)
{

	
	ks_os_thread_create(&thread_handle_tcp_server_task,			
							 "tcp server task",					
							 SocketDemo_SeverRecieveTcp,						
							 0,								
							 15,								
							 thread_stack_tcp_server_task,			
							 sizeof(thread_stack_tcp_server_task),	
							 0,
							 1
							 );
	
	 ks_os_thread_create(&thread_handle_udp_server_task,		 
							  "udp server task",					 
							  SocketDemo_SeverRecieveUdp,						 
							  0,							 
							  15,								 
							  thread_stack_udp_server_task, 		 
							  sizeof(thread_stack_udp_server_task),  
							  0,
							  1
							  );
	 

	ks_shell_add_cmds(socket_demo_cmds, sizeof(socket_demo_cmds) / sizeof(cmd_proc_t));

	
}

