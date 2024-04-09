
#include <stdio.h>
#include <string.h>

#include "lwip/init.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "lwip/memp.h"
#include "lwip/tcpip.h"
#include "lwip/ip_addr.h"
#include "lwip/netif.h"
#include "lwip/timers.h"
#include "netif/etharp.h"
#include "netif/loopif.h"
#include "lwip/sockets.h"

#include "ks_os.h"
#include "ks_taskdef.h"
#include "ks_mem.h"

#if LWIP_DHCP
#include "lwip/dhcp.h"
#endif


#include "arch/sys_arch.h"
#include "lwip/ip_addr.h"
#include "ks_lwip.h"
#include "ks_shell.h"
#include "ks_eth.h"
#include "ipaddr_utils.h"
#include "ks_printf.h"

typedef struct  LwipTaskParam
{
	int psem;
	U32 	ip;
	U16	port;
	lwip_recv_cbk_t cbf;
	void *arg;
	void* socket;
	int ret;
} __attribute__((packed))  LwipTaskParam;



typedef struct  LwipContext
{
    U8  lwip_status;
    LwipNetCfg ethnetcfg;
} __attribute__((packed))  LwipContext;


typedef struct  _MessageHead
{
	U16 msg_id;
	U16 msg_len;
	U16 param;
	U16 result;
} MessageHead;


typedef struct  _MessageBuffer
{
    MessageHead msghead;
    U8  buffer[0];
} __attribute__((packed))MessageBuffer;



struct netif eth_netif;
struct netif loop_netif;

OSHandle thread_eth_lwip_entry;
U8 thread_stack_eth_lwip_entry[1024*4];

OSHandle thread_eth_lwip_dhcp_entry;
U8 thread_stack_eth_lwip_dhcp_entry[1024*4];

LwipContext g_LwipContex;

#define BACKLOG     10


static U32 s_EthLinkFlag;
static OSHandle s_lwipDhcpFlag;


static int init_flag_lwip = 0;

extern err_t ethernetif_init(struct netif *netif);

extern void ethif_recievepacket(U8*buffer,U32 len);
extern void lwip_telentd_Init();
extern void tcpip_loop( );


LwipNetCfg g_defaut_lwip_config = {
	.ipaddr = "192.168.168.100",
	.netmask = "255.255.255.0",
	.gateway = "192.168.168.1"
};


#if 0
void add_ethnetif(void* pcfg)
{
	int uart_id = 0;
	LwipContext* pctx = & g_LwipContex;
	memcpy(&pctx->ethnetcfg, pcfg, sizeof(LwipNetCfg));
	//ks_os_printf(uart_id,"add_ethnetif  ipaddr  %s  \r\n" ,pctx->ethnetcfg.ipaddr );

	LwipNetCfg*  eth =  &pctx->ethnetcfg;
	ip_addr_t ipaddr, netmask, gw;


	IP4_ADDR(&gw, 127,0,0,1);
	IP4_ADDR(&ipaddr, 127,0,0,1);
	IP4_ADDR(&netmask, 255,255,255,0);
	
	eth_netif.hwaddr_len = 6;

	memcpy(loop_netif.hwaddr,ks_driver_eth_get_mac(),6);

	netif_add(&loop_netif, &ipaddr, &netmask, &gw, NULL,loopif_init,tcpip_input);

	
	struct netif *netif = ks_mem_heap_malloc(sizeof(struct netif));
	

	eth_netif.hwaddr_len = 6;
	memcpy(eth_netif.hwaddr,ks_driver_eth_get_mac(),6);

	ipaddr.addr = ipaddr_addr((char*)eth->ipaddr);
	netmask.addr = ipaddr_addr((char*)eth->netmask);
	gw.addr = ipaddr_addr((char*)eth->gateway);
	ks_os_printf(uart_id,"add_ethnetif  ip_addr: %s   netmask:  %s  gw: %s \r\n",eth->ipaddr, eth->netmask,eth->gateway);

	/* Add netif interface for eth  */
	netif_add(&eth_netif,  &ipaddr, &netmask, &gw, NULL, ethernetif_init,tcpip_input);

	netif_set_default(&eth_netif);
	
	netif_set_up(&eth_netif);

	ks_mem_pool_free(pcfg);
}

#endif

void update_ethnetif(void* pcfg)
{
    LwipContext* pctx = & g_LwipContex;
    memcpy(&pctx->ethnetcfg, pcfg, sizeof(LwipNetCfg));
    //ks_os_printf(0,"update_ethnetif  ipaddr  %s  \r\n" ,pctx->ethnetcfg.ipaddr  );

    LwipNetCfg*  eth =  & pctx->ethnetcfg;
    ip_addr_t ipaddr, netmask, gw;
    
    
	eth_netif.hwaddr_len = 6;
	memcpy(eth_netif.hwaddr,ks_driver_eth_get_mac(),6);

	
    ipaddr.addr = ipaddr_addr((char*)eth->ipaddr);
    netmask.addr = ipaddr_addr((char*)eth->netmask);
    gw.addr = ipaddr_addr((char*)eth->gateway);


	ks_os_printf(0,"update_ethnetif ip_addr: %s   netmask:  %s  gw: %s \r\n",eth->ipaddr, eth->netmask,eth->gateway);

    netif_set_down(&eth_netif);
    
  
    netif_set_gw(&eth_netif, &gw);       
    netif_set_netmask(&eth_netif, &netmask); 
    netif_set_ipaddr(&eth_netif, &ipaddr);    

 
    netif_set_up(&eth_netif);
	
    ks_mem_pool_free(pcfg);
}


void setup_ethnetif()
{
	int uart_id = 0;
	LwipContext* pctx = & g_LwipContex;
	//ks_os_printf(uart_id,"add_ethnetif  ipaddr  %s  \r\n" ,pctx->ethnetcfg.ipaddr );

	LwipNetCfg*  eth =  &pctx->ethnetcfg;
	ip_addr_t ipaddr, netmask, gw;


	IP4_ADDR(&gw, 127,0,0,1);
	IP4_ADDR(&ipaddr, 127,0,0,1);
	IP4_ADDR(&netmask, 255,255,255,0);
	
	eth_netif.hwaddr_len = 6;

	memcpy(loop_netif.hwaddr,ks_driver_eth_get_mac(),6);

	netif_add(&loop_netif, &ipaddr, &netmask, &gw, NULL,loopif_init,tcpip_input);


	eth_netif.hwaddr_len = 6;
	memcpy(eth_netif.hwaddr,ks_driver_eth_get_mac(),6);

	ipaddr.addr = 0;
	netmask.addr = 0;
	gw.addr =0;

	/* Add netif interface for eth  */
	netif_add(&eth_netif,  &ipaddr, &netmask, &gw, NULL, ethernetif_init,tcpip_input);

	netif_set_default(&eth_netif);
	
	netif_set_up(&eth_netif);

}


#if 0
int  ks_lwip_add_eth_net_intf(LwipNetCfg* param)
{
    LwipNetCfg*  pethcfg = ks_mem_pool_malloc( sizeof(LwipNetCfg));
    if(pethcfg!=NULL){
        memcpy(pethcfg, param, sizeof(LwipNetCfg));
       return  tcpip_callback_with_block(add_ethnetif, pethcfg, 0);
    }

    return -1;
}
#endif

int ks_lwip_update_eth_net_intf(LwipNetCfg* param)
{
    LwipNetCfg*  pethcfg = ks_mem_pool_malloc( sizeof(LwipNetCfg));
    if(pethcfg!=NULL){
        memcpy(pethcfg, param, sizeof(LwipNetCfg));
       return  tcpip_callback_with_block(update_ethnetif, pethcfg, 0);
    }

    return -1;
}


int ks_lwip_upate_eth_netcfg(LwipNetCfg* param){

	return ks_lwip_update_eth_net_intf(param);

}



int Lwip_SetEthLinkUp(){
    return tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_up,
					(void *) &eth_netif, 1);
}

int Lwip_SetEthLinkDown(){
return tcpip_callback_with_block((tcpip_callback_fn) netif_set_link_down,
					(void *) &eth_netif, 1);
}



static void udp_server_thread(void *arg)
{  
	int uart_id = 0;


	LwipTaskParam* param = (LwipTaskParam* ) arg;
	lwip_recv_cbk_t udp_recv_cbk =param->cbf;
	void *argout = param->arg;
	OSHandle* socket_out =param->socket;
	U16 localport = param->port;

	
    int iRecvLen;
    U8 ucRecvBuf[BUFFER_LEN];

	err_t err;
	struct netconn *udpconn;
	struct netbuf  *recvbuf;
	struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	U32 data_len = 0,rec_len = 0;
	struct pbuf *q;
	int cnt=0;

	LWIP_UNUSED_ARG(arg);
	udpconn = netconn_new(NETCONN_UDP); 
	
	if(udpconn == NULL){
		ks_os_printf(uart_id,"netconn_new error!\r\n");
		param->ret = -1;
		param->psem = 0;
		goto errloop;
	}

	//udpconn->recv_timeout = 10; 		
	err = netconn_bind(udpconn,IP_ADDR_ANY,localport); 
    if (err != ERR_OK)
    {
    	ks_os_printf(uart_id,"bind error!\r\n");
		param->ret = -2;
		param->psem = 0;
		goto errloop;
    }


	param->ret = 0;
	if(socket_out!=NULL){
		//ks_os_printf(uart_id,"socket_out %x \r\n",udpconn);
		(*socket_out) = (OSHandle)udpconn;
	}
	
	param->psem = 0;

	while(1)
	{
		netconn_recv(udpconn,&recvbuf); 
		if(recvbuf != NULL) 	
		{ 
			memset(ucRecvBuf,0,sizeof(ucRecvBuf));  //数据接收缓冲区清零
			for(q=recvbuf->p;q!=NULL;q=q->next)  //遍历完整个pbuf链表
			{
				if(q->len > (sizeof(ucRecvBuf)-data_len)) memcpy(ucRecvBuf+data_len,q->payload,(sizeof(ucRecvBuf)-data_len));//拷贝数据
				else memcpy(ucRecvBuf+data_len,q->payload,q->len);
				data_len += q->len; 	
				if(data_len > sizeof(ucRecvBuf)) break; 
			}
			iRecvLen = data_len;
			data_len=0;  

			//ks_os_printf(uart_id,"Recv Msg From %s	port %d  \r\n", iputils_ultoa(ntohl(recvbuf->addr.addr)), recvbuf->port);
			if(udp_recv_cbk!=NULL){
				udp_recv_cbk((OSHandle)udpconn, localport,ucRecvBuf,iRecvLen,ntohl(recvbuf->addr.addr),recvbuf->port,argout);
			}

			//netconn_sendto(udpconn,recvbuf,&recvbuf->addr,recvbuf->port);
			//ks_os_printf(uart_id,"%s\r\n",ucRecvBuf);	
			netbuf_delete(recvbuf);
		}
	}


errloop:
	
	while(1){
		ks_os_thread_sleep_msec(1000);
	}

}


int  ks_lwip_udp_server_create( U16	udp_listen_port  ,lwip_recv_cbk_t cbf,void* arg,OSHandle* socket_out){

	int uart_id = 0;
    if(g_LwipContex.lwip_status == 0){
		return -1;
	}
	

	LwipTaskParam param;
	//赋初值
	if(socket_out!=NULL){
		(*socket_out)= -1;
	}

	param.psem = 1;
	param.port = udp_listen_port;
	param.cbf = cbf;
	param.arg = arg;
	param.socket =socket_out;


	OSHandle thread_handle ;
	U8* pthread_name = ks_mem_heap_malloc(30);
	memset(pthread_name,0,30);
	sprintf((char*)pthread_name,"udp server %d ",udp_listen_port);
	U8* pthread_stack = ks_mem_heap_malloc(1024*4);

	int ret = ks_os_thread_create(&thread_handle,			//	thread_handle
					(char*)pthread_name,					//	thread_name
	    			 udp_server_thread,						//	thread_entry
	    			  &param, 									//	arg
	    			 _THREAD_PRI_LWIP_SERVER,					 //	priority
	    			 pthread_stack,			//	stack_start
	    			 1024*4,	//	stack_size
					 0,
					 1
					 );

	if(ret == OSHANDLE_SUCCESS){

		while(param.psem)ks_os_thread_sleep_msec(10);
			
		return param.ret;

	}else{
		ks_mem_heap_free(pthread_stack);
		ks_mem_heap_free(pthread_name);
	}

	return -2;
	
}




int  ks_lwip_udp_server_send(OSHandle sock, U32 dstip, U16 dstport, U8*pbuf,U32 tlen)
{

	///if(s_EthLinkFlag==0  || g_LwipContex.lwip_status == 0) return -1;
	if( g_LwipContex.lwip_status == 0) return -1;


	if(sock == 0 ) return -2;
	if(tlen == 0 ) return 0;
	
	ip_addr_t dst_addr ;
    dst_addr.addr = htonl(dstip);

	struct netbuf  *sentbuf = netbuf_new();
	if(sentbuf==NULL) return -1;
	U8* pbuffer = netbuf_alloc(sentbuf,tlen);
	if(pbuffer==NULL) return -2;
	memcpy(pbuffer,(void*)pbuf,tlen);

    //ks_os_printf(0,"ks_lwip_udp_send sock %x dst_ip %s port %d  tlen %d !\r\n" ,sock,iputils_ultoa(dstip),dstport,tlen);
	//ks_os_printf(uart_id," %s  !\r\n" ,pbuf);
	int err = netconn_sendto((struct netconn *)sock,sentbuf,&dst_addr,dstport);
	if(err != ERR_OK)
	{
		netbuf_delete(sentbuf); 	
		ks_os_printf(0," %d  !\r\n" ,err);
		return -3;
	}
	netbuf_delete(sentbuf); 		
	return tlen;


}


int  ks_lwip_udp_poll_send(U32 dstip, U16 dstport, U8*pbuf,U32 tlen)
{

	///if(s_EthLinkFlag==0  || g_LwipContex.lwip_status == 0) return -1;
	if( g_LwipContex.lwip_status == 0) return -1;

	struct netconn *udpconn = netconn_new(NETCONN_UDP);				
	if(udpconn == NULL){
		return -1;
	}


	if(tlen == 0 ) return 0;
	
	ip_addr_t dst_addr ;
    dst_addr.addr = htonl(dstip);

	struct netbuf  *sentbuf = netbuf_new();
	if(sentbuf==NULL) return -1;
	U8* pbuffer = netbuf_alloc(sentbuf,tlen);
	if(pbuffer==NULL) return -2;
	memcpy(pbuffer,(void*)pbuf,tlen);

    ks_os_printf(0,"ks_lwip_udp_client_send sock %x dst_ip %s port %d  tlen %d !\r\n" ,udpconn,iputils_ultoa(dstip),dstport,tlen);
	//ks_os_printf(uart_id," %s  !\r\n" ,pbuf);
	int err = netconn_sendto(udpconn,sentbuf,&dst_addr,dstport);
	if(err != ERR_OK)
	{	
		ks_os_printf(0," %d  !\r\n" ,err);
		return -3;
	}

	netbuf_delete(sentbuf); 
	netconn_delete(udpconn);
	return tlen;


}



int  ks_lwip_udp_client_send(OSHandle clienthandle, U8*pbuf,U32 len)
{

	MessageBuffer*	pmsg = ks_mem_heap_malloc(sizeof(MessageBuffer)+len);
	if(pmsg!=NULL){
		pmsg->msghead.msg_id = 1;
		pmsg->msghead.msg_len = len;
		memcpy(pmsg->buffer,pbuf,len);
		ubase_t ptr = (ubase_t)pmsg;
		return ks_os_queue_send((OSHandle)clienthandle,&ptr,KS_OS_NO_WAIT);
	}
	return -1;
}


static void udp_client_thread(void *arg)
{
	int uart_id = 0;
	MessageBuffer*  pmsg ;
	OSHandle task_msg_queue;
	U32 status;

	LwipTaskParam* param = (LwipTaskParam* ) arg;

	U16 dstport = param->port;
	U32 dstip = param->ip;

	err_t err;
	struct netconn *udpconn;
	struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	int data_len = 0,rec_len = 0;
	struct pbuf *q;
	int cnt=0;

	LWIP_UNUSED_ARG(arg);
	udpconn = netconn_new(NETCONN_UDP);  
	
	if(udpconn == NULL){
		ks_os_printf(uart_id,"netconn_new error!\r\n");
		param->ret = -1;
		param->psem = 0;
		goto errloop;
	}

	ip_addr_t dst_addr ;
    dst_addr.addr = htonl(dstip);

	param->ret = 0;
	param->psem = 0;
	task_msg_queue =(OSHandle) param->arg;

	while(1)
	{
        status = ks_os_queue_receive(task_msg_queue,&pmsg,KS_OS_WAIT_FOREVER);
		if(status == 0)
		{
			int tlen = pmsg->msghead.msg_len;
			
			struct netbuf  *sentbuf = netbuf_new();
			
			if(sentbuf != NULL) 
			{
				U8* pbuffer = netbuf_alloc(sentbuf,tlen);
				if(pbuffer != NULL) 
				{
					memcpy(pbuffer,(void*)pmsg->buffer,tlen);
					//ks_os_printf(0,"ks_lwip_udp_client_send sock %x dst_ip %s port %d  tlen %d !\r\n" ,udpconn,iputils_ultoa(dstip),dstport,tlen);
					int err = netconn_sendto(udpconn,sentbuf,&dst_addr,dstport);
					if(err != ERR_OK)
					{	
						ks_os_printf(0," %d  !\r\n" ,err);
					}

					netbuf_delete(sentbuf); 
				};
			};
		
			ks_mem_heap_free(pmsg);

		}else{
			ks_os_printf(uart_id,"ks_os_queue_receive err %x \r\n",status);
		}

	}

errloop:
	
	while(1){
		ks_os_thread_sleep_msec(1000);
	}

	
}


int  ks_lwip_udp_client_create( U32 dstip,U16	udp_port  ,OSHandle * clienthandle){

    if(g_LwipContex.lwip_status == 0){
		return -1;
	}
	
	LwipTaskParam param;
	
	param.psem = 1;
	param.ip = dstip;
	param.port = udp_port;

	OSHandle thread_handle ;
	U8* pthread_name = ks_mem_heap_malloc(30);
	memset(pthread_name,0,30);
	sprintf((char*)pthread_name,"udp client %d ",udp_port);
	U8* pthread_stack = ks_mem_heap_malloc(1024*4);
	OSHandle task_msg_queue;
	
	ks_os_queue_create(&task_msg_queue,(char*)pthread_name,32,sizeof(void*));

	*clienthandle = (OSHandle) task_msg_queue;
	param.arg = (void *)task_msg_queue;

	
	int ret =ks_os_thread_create(&thread_handle,			//	thread_handle
	    			 (char*)pthread_name,					//	thread_name
	    			 udp_client_thread,						//	thread_entry
	    			  &param, 									//	arg
	    			 _THREAD_PRI_LWIP_CLIENT,					 //	priority
	    			 pthread_stack,			//	stack_start
	    			 1024*4,	//	stack_size
					 0,
					 1
					 );


	if(ret == OSHANDLE_SUCCESS){
		while(param.psem)ks_os_thread_sleep(10);
	
		return param.ret;

	}else{
			ks_mem_heap_free(pthread_stack);
			ks_mem_heap_free(pthread_name);
			ks_os_queue_delete(task_msg_queue);
	}

	return -2;
	
}


static void tcp_server_thread(void *arg)
{  
	int uart_id = 0;
	LwipTaskParam* param = (LwipTaskParam* ) arg;
	lwip_recv_cbk_t tcp_recv_cbk =param->cbf;
	void *argout = param->arg;
	void* socket_out = param->socket;
	int iRecvLen;
	U8 ucRecvBuf[BUFFER_LEN];
	U16 localport = param->port;

	U32 data_len = 0;
	struct pbuf *q;
	err_t err,recv_err;
	struct netconn *conn, *newconn;
	ip_addr_t ipaddr;
	u16_t remote_port;

	conn = netconn_new(NETCONN_TCP);  
	if(conn==NULL){
		param->ret =-1;
		param->psem = 0;
		goto errloop;

	}
	netconn_bind(conn,IP_ADDR_ANY,localport);
	netconn_listen(conn);	
	//超时时间的设定 无论TCP断线重连还是网线断线重连，都可以恢复通信
	conn->recv_timeout = 3000;	

	param->ret = 0;
	param->psem = 0;

	while (1) 
	{
		err = netconn_accept(conn,&newconn);  
		if(err==ERR_OK)newconn->recv_timeout = 10;

		if (err == ERR_OK)	 
		{ 
			struct netbuf *recvbuf;

			netconn_getaddr(newconn,&ipaddr,&remote_port,0); 

			if(socket_out!=NULL){
				(*(OSHandle*)socket_out) = (OSHandle)newconn;
			}

			ks_os_printf(uart_id,"get connect from : %s  %d \r\n", inet_ntoa(ipaddr),remote_port);
			while(1)
			{

				if((recv_err = netconn_recv(newconn,&recvbuf)) == ERR_OK)  
				{		

					memset(ucRecvBuf,0,sizeof(ucRecvBuf));  
					for(q=recvbuf->p;q!=NULL;q=q->next)  
					{
						if(q->len > (sizeof(ucRecvBuf)-data_len)) memcpy(ucRecvBuf+data_len,q->payload,(sizeof(ucRecvBuf)-data_len));//
						else memcpy(ucRecvBuf+data_len,q->payload,q->len);
						data_len += q->len; 	
						if(data_len > sizeof(ucRecvBuf)) break;
					}
					
					iRecvLen = data_len;
					data_len=0;  
				
					if(tcp_recv_cbk!=NULL){
						tcp_recv_cbk((OSHandle)newconn, localport,ucRecvBuf,iRecvLen,ntohl(ipaddr.addr),remote_port,argout);
					}

					netbuf_delete(recvbuf);
			
				}
				else if(recv_err == ERR_CLSD) 
				{
					netconn_close(newconn);
					netconn_delete(newconn);
					
					if(socket_out!=NULL){
						(*(int*)socket_out) = -1;
					}
					ks_os_printf(uart_id,"close : %s  %d \r\n", inet_ntoa(ipaddr),remote_port);
					break;
				}
			}
		}
		else{
			//ks_os_printf(uart_id,"netconn_accept %x \r\n",err);
		}
	}


errloop:
	
	while(1){
		ks_os_thread_sleep_msec(1000);
	}

}

int  ks_lwip_tcp_server_create( U16 tcp_listen_port  ,lwip_recv_cbk_t cbf,void *arg,OSHandle * socket_out){

    if(g_LwipContex.lwip_status == 0){
		return -1;
	}
	
	LwipTaskParam param;
	
	if(socket_out!=NULL){
		(*socket_out)= -1;
	}

	param.psem = 1;
	param.port = tcp_listen_port;
	param.cbf = cbf;
	param.arg = arg;
	param.socket =socket_out;

	OSHandle thread_handle ;
	U8* pthread_name = ks_mem_heap_malloc(30);
	memset(pthread_name,0,30);
	sprintf((char*)pthread_name,"tcp server %d ",tcp_listen_port);
	U8* pthread_stack = ks_mem_heap_malloc(1024*4);
	
	int ret =ks_os_thread_create(&thread_handle,			//	thread_handle
					(char*)pthread_name,					//	thread_name
	    			 tcp_server_thread,						//	thread_entry
	    			  &param, 									//	arg
	    			 _THREAD_PRI_LWIP_SERVER,					 //	priority
	    			 pthread_stack,			//	stack_start
	    			 1024*4,	//	stack_size
					 0,
					 1
					 );


	if(ret == OSHANDLE_SUCCESS){
		while(param.psem)ks_os_thread_sleep(10);
	
		return param.ret;
	}
	else{
		ks_mem_heap_free(pthread_stack);
		ks_mem_heap_free(pthread_name);
	}

	return -2;
	
}




int  ks_lwip_tcp_server_send(OSHandle sock, U8*pbuf,U32 tlen){
	int err = -1;
	if(s_EthLinkFlag==0 || g_LwipContex.lwip_status == 0) return -1;

	if(sock == 0 ) return -2;
	
	if(tlen == 0 ) return 0;

	err = netconn_write((struct netconn *)sock,pbuf,tlen,NETCONN_COPY);
	if(err != ERR_OK)
	{
		kprintf("netconn_write err %d \r\n",err);
		return err;
	}
	return tlen;
}




int  ks_lwip_tcp_poll_send(U32 dstip, U16 dstport, U8*pbuf,U32 tlen){

	struct netconn *tcp_clientconn;
	err_t err,recv_err;
	//if(s_EthLinkFlag==0 || g_LwipContex.lwip_status == 0) return -1;
	if(g_LwipContex.lwip_status == 0) return -1;

	ip_addr_t server_ipaddr ;
    server_ipaddr.addr = htonl(dstip);
	
	if(tlen == 0 ) return 0;


	tcp_clientconn = netconn_new(NETCONN_TCP);	
	err = netconn_connect(tcp_clientconn,&server_ipaddr,dstport);
	if(err != ERR_OK)  netconn_delete(tcp_clientconn);
	else if (err == ERR_OK)   
	{ 
		err = netconn_write(tcp_clientconn,pbuf,tlen,NETCONN_COPY);
		if(err != ERR_OK)
		{
			kprintf("netconn_write err %d \r\n",err);
		}

	}

	netconn_close(tcp_clientconn);
	netconn_delete(tcp_clientconn);
	return 0;
}


int  ks_lwip_tcp_client_send(OSHandle clienthandle, U8*pbuf,U32 len)
{

	MessageBuffer*	pmsg = ks_mem_heap_malloc(sizeof(MessageBuffer)+len);
	if(pmsg!=NULL){
		pmsg->msghead.msg_id = 1;
		pmsg->msghead.msg_len = len;
		memcpy(pmsg->buffer,pbuf,len);
		ubase_t ptr = (ubase_t)pmsg;
		return ks_os_queue_send(clienthandle,&ptr,KS_OS_NO_WAIT);
	}
	return -1;
}



static void tcp_client_thread(void *arg)
{

	int uart_id = 0;
	MessageBuffer*  pmsg ;
	OSHandle task_msg_queue;
	U32 status;

	LwipTaskParam* param = (LwipTaskParam* ) arg;

	U16 dstport = param->port;
	U32 dstip = param->ip;

	err_t err;
	struct netconn *tcp_clientconn;
	struct netbuf  *sentbuf;
	struct ip_addr destipaddr;
	int data_len = 0,rec_len = 0;
	struct pbuf *q;
	int cnt=0;

	tcp_clientconn = netconn_new(NETCONN_TCP);	

	if(tcp_clientconn == NULL){
		ks_os_printf(uart_id,"netconn_new error!\r\n");
		param->ret = -1;
		param->psem = 0;
		goto errloop;
	}

	ip_addr_t dst_addr ;
    dst_addr.addr = htonl(dstip);
	
	err = netconn_connect(tcp_clientconn,&dst_addr,dstport);
	if(err != ERR_OK) {
		ks_os_printf(uart_id,"netconn_connect error!\r\n");
		netconn_delete(tcp_clientconn); //返回值不等于ERR_OK,删除tcp_clientconn连接
		param->ret = -2;
		param->psem = 0;
		goto errloop;

	} 

	param->ret = 0;
	param->psem = 0;
	task_msg_queue = (OSHandle)param->arg;


	while(1)
	{
		status = ks_os_queue_receive(task_msg_queue,&pmsg,KS_OS_WAIT_FOREVER);
		if(status == 0)
		{
		   int tlen = pmsg->msghead.msg_len;
		   
		   err = netconn_write(tcp_clientconn,pmsg->buffer,tlen,NETCONN_COPY);
		   if(err != ERR_OK)
		   {
			   ks_os_printf(uart_id,"netconn_write err %d \r\n",err);
		   }

		   ks_mem_heap_free(pmsg);

		}else{
		   ks_os_printf(uart_id,"ks_os_queue_receive err %x \r\n",status);
		}

	}


	
errloop:
	
	while(1){
		ks_os_thread_sleep_msec(1000);
	}
}


int  ks_lwip_tcp_client_create( U32 dstip,U16	tcp_port  ,OSHandle * clienthandle){

    if(g_LwipContex.lwip_status == 0){
		return -1;
	}
	
	LwipTaskParam param;

	param.psem = 1;
	param.ip = dstip;
	param.port = tcp_port;


	OSHandle thread_handle ;
	U8* pthread_name = ks_mem_heap_malloc(30);
	memset(pthread_name,0,30);
	sprintf((char*)pthread_name,"tcp client %d ",tcp_port);
	U8* pthread_stack = ks_mem_heap_malloc(1024*4);
	
	OSHandle task_msg_queue;
	ks_os_queue_create(&task_msg_queue,(char*)pthread_name,32,sizeof(void*));

	*clienthandle = (OSHandle) task_msg_queue;
	param.arg = (void*)task_msg_queue;
	
	int ret =ks_os_thread_create(&thread_handle,			//	thread_handle
					(char*)pthread_name,					//	thread_name
	    			 tcp_client_thread,						//	thread_entry
	    			  &param, 									//	arg
	    			 _THREAD_PRI_LWIP_CLIENT,					 //	priority
	    			 pthread_stack,			//	stack_start
	    			 1024*4,	//	stack_size
					 0,
					 1
					 );


	if(ret == OSHANDLE_SUCCESS){
		while(param.psem)ks_os_thread_sleep(10);
		return param.ret;
	}else{
		ks_mem_heap_free(pthread_stack);
		ks_mem_heap_free(pthread_name);
		ks_os_queue_delete(task_msg_queue);
	}

	return -2;
	
}


/* Callback for TCPIP thread to indicate TCPIP init is done */
static void tcpip_init_done_signal(void *arg)
{
    /* Tell main thread TCP/IP init is done */
	if(s_EthLinkFlag != 0){
		Lwip_SetEthLinkUp();
	}

    g_LwipContex.lwip_status = 1;
	
	int* tcpipdone = ( int* )arg;
	
	*tcpipdone = 1;

	//ks_os_printf(0, "g_LwipContex.lwip_status == 1 \r\n");

}



void Lwip_TaskMainLoop(void *arg)
{

    struct netif *netif;
    tcpip_init(tcpip_init_done_signal,  arg);

	ks_os_printf(0,"Lwip_TaskMainLoop enter tcpip_loop \r\n");

	setup_ethnetif();

    //enter loop 
    tcpip_loop();

}

#if LWIP_DHCP	

#define LWIP_MAX_DHCP_TRIES		100  //DHCP服务器最大重试次数
   
void Lwip_TaskDhcp(void *arg)
{
	struct netif* dhcp_netif;				//定义一个全局的网络接口

	LwipNetCfg param;
	dhcp_netif = &eth_netif;


	memset(&param,0,sizeof(LwipNetCfg));
	
	
	while(1){

		ks_os_flag_wait(s_lwipDhcpFlag, 1, WAIT_FLAG_MODE_OR_CLEAR,KS_OS_WAIT_FOREVER);
		
		if(s_EthLinkFlag==0  || g_LwipContex.lwip_status == 0) {
			continue;
		}

		if(!(dhcp_netif->flags & NETIF_FLAG_DHCP)){
			ks_os_printf(0,"dhcp_start\r\n");
			dhcp_start(dhcp_netif);//开启DHCP 
		}
		

		while(1)
		{ 
		
			if(dhcp_netif->dhcp!= NULL) //通过DHCP服务获取IP地址失败,且超过最大尝试次数
			{
				if(dhcp_netif->dhcp->tries>LWIP_MAX_DHCP_TRIES){
					ks_os_printf(0,"dhcp failure \r\n");
					break;
				}
				
				ks_os_printf(0,"dhcp state %d  \r\n",dhcp_netif->dhcp->state);
			}
			
			if(dhcp_netif->ip_addr.addr!=0)	 {
				ks_os_printf(0,"dhcp get eth info success! \r\n");
				char* ip = ip_ntoa(&dhcp_netif->ip_addr);
				memcpy(param.ipaddr,ip,strlen(ip));
				ks_os_printf(0,"ip_addr:%s	\r\n",ip);
				
				char*  netmask= ip_ntoa(&dhcp_netif->netmask);
				ks_os_printf(0,"netmask:%s	\r\n",netmask);
				memcpy(param.netmask,netmask,strlen(netmask));
				
				char*  gw= ip_ntoa(&dhcp_netif->gw);
				ks_os_printf(0,"gateway:%s	\r\n",gw);
				memcpy(param.gateway,gw,strlen(gw));

				
				//ks_lwip_update_eth_net_intf(&param);
				break;
			}		
			

			ks_os_thread_sleep_msec(3000);
			
		}

		dhcp_stop(dhcp_netif); 		//关闭DHCP

	}

}

#endif

void Lwip_StatusNotify(     U32 linkflag, U32 linkspeed)
{
	s_EthLinkFlag = linkflag;
	ks_os_printf(0,"Lwip_StatusNotify  linkflag %d  linkspeed %d   \r\n",linkflag,linkspeed);

	if(s_EthLinkFlag != 0){
#if LWIP_DHCP	
		if(s_lwipDhcpFlag!=0){
			ks_os_flag_set_bits(s_lwipDhcpFlag, 1, SET_FLAG_MODE_OR);
		}
#endif
		Lwip_SetEthLinkUp();
	}

}



int  ifconfig(cmd_proc_t* ctx,int argc,  char **argv ){


    if ( argc >= 2 )
    {
		if(ip_isvalued(argv[1]) == 1){
			ks_os_printf(ctx->uart_id, "invalued <ip> <netmask> <gateway>\n", argv[1] );
			goto usage;
		}
		LwipNetCfg param;

		strcpy((char*)param.ipaddr, argv[1]);

		if(argc >= 3){
			strcpy((char*)param.netmask,argv[2]);
		}else{
			strcpy((char*)param.netmask,ip_ntoa(&eth_netif.netmask));
		}
		if(argc >= 4){
			strcpy((char*)param.gateway,argv[3]);
		}else{
			strcpy((char*)param.gateway,ip_ntoa(&eth_netif.gw));
		}

		ks_lwip_update_eth_net_intf(&param);
    }
    
	ks_os_thread_sleep_msec(100);
	
    ks_os_printf(ctx->uart_id,"\r\n**************************************************\r\n");
    ks_os_printf(ctx->uart_id, "ifconfig\r\n");

  	struct netif *netif = netif_list;
    while (netif != NULL) {
       ks_os_printf(ctx->uart_id, "%s: flags= %d mtu  %d \r\n", netif->name,netif->flags,netif->mtu);
       ks_os_printf(ctx->uart_id,"    ip_addr:%s  \r\n",ip_ntoa(& netif->ip_addr));
       ks_os_printf(ctx->uart_id,"    netmask:%s  \r\n",ip_ntoa( &netif->netmask));
       ks_os_printf(ctx->uart_id,"    gw:%s \r\n",ip_ntoa(& netif->gw));
       ks_os_printf(ctx->uart_id,"    mac:%s \r\n", mac_htoa(netif->hwaddr));
       ks_os_printf(ctx->uart_id, "\r\n");
       netif = netif->next;
    }
    ks_os_printf(ctx->uart_id,"\r\n***************************************************\r\n");

	return 0;
	
usage:
	   printf("usage: %s <ip> <netmask> <gateway> \r\n", argv[0] );	 

	return 0;
}

int  dhcp(cmd_proc_t* ctx,int argc,  char **argv ){

    ks_os_printf(ctx->uart_id, "dhcp\r\n");
	

#if LWIP_DHCP	
	if(s_lwipDhcpFlag!=0){
		eth_netif.ip_addr.addr = 0;
		ks_os_flag_set_bits(s_lwipDhcpFlag, 1, SET_FLAG_MODE_OR);
	}
#endif
	
	return 0;
}

static cmd_proc_t lwip_cmds[] = {
    {.cmd = "ifconfig", .fn = ifconfig,  .help = "ifconfig <ip> <netmask> <gateway>"},
#if LWIP_DHCP	
	{.cmd = "dhcp", .fn = dhcp,  .help = "dhcp ip"},
#endif

};


void ks_lwip_init(LwipNetCfg*  pNetCfg )
{

	if(init_flag_lwip == 0){


		int  tcpipdone = 0 ;

		ks_os_thread_create(&thread_eth_lwip_entry,			//	thread_handle
	    			 "eth lwip task",					//	thread_name
	    			 Lwip_TaskMainLoop,						//	thread_entry
	    			  &tcpipdone, 								//	arg
	    			 _THREAD_PRI_LWIP_MAIN,					 //	priority
	    			 thread_stack_eth_lwip_entry,			//	stack_start
	    			 sizeof(thread_stack_eth_lwip_entry),	//	stack_size
					 0,
					 1
					 );


		while(!tcpipdone)ks_os_thread_sleep_msec(10);

		
		if(pNetCfg != NULL){
			ks_lwip_update_eth_net_intf(pNetCfg);
		}else{
#if LWIP_DHCP	
			ks_os_flag_create(&s_lwipDhcpFlag, "lwip Dhcp Flag");
			ks_os_thread_create(&thread_eth_lwip_dhcp_entry,			//	thread_handle
								 "eth lwip dhcp task",					//	thread_name
								 Lwip_TaskDhcp, 					//	thread_entry
								  NULL, 								//	arg
								 _THREAD_PRI_LWIP_SERVER,					 // priority
								 thread_stack_eth_lwip_dhcp_entry,			//	stack_start
								 sizeof(thread_stack_eth_lwip_dhcp_entry),	//	stack_size
								 0,
								 1
								 );

		 	ks_os_flag_set_bits(s_lwipDhcpFlag, 1, SET_FLAG_MODE_OR);
#endif

		}
	
		ks_driver_eth_add_data_recv_callback (ethif_recievepacket);
		
		ks_driver_eth_add_status_change_callback(Lwip_StatusNotify);
		
		ks_shell_add_cmds(lwip_cmds, sizeof(lwip_cmds) / sizeof(cmd_proc_t));


		init_flag_lwip = 1;
	}

}


