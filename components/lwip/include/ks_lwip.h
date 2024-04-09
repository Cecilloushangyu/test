
#ifndef __LWIP_INIT_H_
#define __LWIP_INIT_H_

#include "ks_datatypes.h"

#include <ks_os.h>

#define BUFFER_LEN     1500


#define CFG_ETH_STR_LEN 16 


typedef struct
{
    U8 ipaddr[CFG_ETH_STR_LEN];
    U8 netmask[CFG_ETH_STR_LEN];
    U8 gateway[CFG_ETH_STR_LEN];
} LwipNetCfg;





/**
*                                        
* Description:  tcp udp 数据接收回调定义
* Arguments  :  sock:  tcp : accept 返回的    socket id   udp : 本地创建的 socketid
*				pbuf：发送数据buffer
*				tlen: 发送数据长度
				srcip: 发送方 IP  本地字节序
				srcport ： 发送方 端口号 本地字节序
*
* Returns    : 		
* Note(s)    :  			
**/

typedef void (*lwip_recv_cbk_t)( OSHandle sock, U16 localport, U8 * buffer,S32 len, U32 srcip, U16 srcport, void * parg );






/**
*                                      ks_lwip_udp_server_create  
*
* Description:  创建udp server函数, 实现udp数据接收

* Arguments  :  udp_listen_port: udp 监听端口
*				cbf：UPD 数据接收回调函数
*				arg: 附带输入参数，作为接收回调函数 cbf 形参
				socket_out： 输出返回  > 0 有效   < 0 无效
*
* Returns    :  函数调用返回结果：成功返回  0    失败返回 < 0
				
* Note(s)    :  函数通过创建线程实现功能
				
**/

S32  ks_lwip_udp_server_create( U16	udp_listen_port  ,lwip_recv_cbk_t cbf,void *arg,OSHandle* socket_out);





/**
*                                      ks_lwip_tcp_server_create
*
* Description:  创建tcp server函数, 实现tcp数据接收

* Arguments  :  tcp_listen_port: tcp 监听端口
*				cbf：TCP 数据接收回调函数
				arg: 附带输入参数，作为接收回调函数 cbf 形参
				socket_out： 输出返回  > 0 有效   < 0 无效
*
* Returns    :  函数调用返回结果：成功返回  0  失败返回 < 0
* Note(s)    :  函数通过创建线程实现功能
				
**/

S32  ks_lwip_tcp_server_create( U16 tcp_listen_port  ,lwip_recv_cbk_t cbf,void *arg,OSHandle* socket_out);





/**
*                                      ks_lwip_udp_client_create  
*
* Description:  udp client 数据发送 创建函数

* Arguments  :  
*				dstip：发送目的IP     本地字节序
*				dstport：发送目的端口号，本地字节序
				clienthandle 输出返回  > 0 有效   < 0 无效
*
* Returns    :  函数调用返回结果：成功返回  0    失败返回 < 0
				
* Note(s)    :  函数通过创建线程实现功能，udp端口创建后不会释放
				
**/

S32  ks_lwip_udp_client_create( U32 dstip,U16	dstport  ,OSHandle * clienthandle);



/**
*                                      ks_lwip_tcp_client_create  
*
* Description:  tcp client 数据发送 创建函数

* Arguments  :  
*				dstip：发送目的IP     本地字节序
*				dstport：发送目的端口号，本地字节序
				clienthandle 输出返回  > 0 有效   < 0 无效
*
* Returns    :  函数调用返回结果：成功返回  0    失败返回 < 0
				
* Note(s)    :  函数通过创建线程实现功能，tcp端口创建后不会释放
				
**/

S32  ks_lwip_tcp_client_create( U32 dstip,U16	dstport  ,OSHandle * clienthandle);


/**
*                                      ks_lwip_add_eth_net_intf  
*
* Description:  添加以太网接口配置
* Arguments  :  
* Returns    :  			
* Note(s)    :  
**/

S32 ks_lwip_add_eth_net_intf(LwipNetCfg* param);



/**

*                                      Lwip_UpdateEthNetIntf  
*
* Description:  更新以太网接口配置
* Arguments  :  
*				 
*
* Returns    :  
				
* Note(s)    :  
				
**/

S32 ks_lwip_update_eth_net_intf(LwipNetCfg* param);


/**
*                                      ks_lwip_udp_client_send  
*
* Description:  udp  client 发送数据接口

* Arguments  :  
*				clienthandle 输出返回  > 0 有效   < 0 无效
*				pbuf：发送数据buffer
*				tlen: 发送数据长度
*           
*
* Returns    :   函数调用返回结果：成功 返回 socket id > 0  失败返回 < 0
				

* Note(s)    :  函数通过创建线程实现功能
				
**/

S32  ks_lwip_udp_client_send(OSHandle clienthandle, U8*pbuf,U32 len);


/**
*                                      ks_lwip_tcp_client_send  
*
* Description:  tcp  client 发送数据接口

* Arguments  :  
*				clienthandle 输出返回  > 0 有效   < 0 无效
*				pbuf：发送数据buffer
*				tlen: 发送数据长度
*           
*
* Returns    :   函数调用返回结果：成功 返回 socket id > 0  失败返回 < 0
				

* Note(s)    :  函数通过创建线程实现功能
**/

S32  ks_lwip_tcp_client_send(OSHandle clienthandle, U8*pbuf,U32 len);


/**
*                                      ks_lwip_tcp_poll_send  
*
* Description:  tcp发送数据接口

* Arguments  :  
*				dstip：发送目的IP     本地字节序
*				dstport：发送目的端口号，本地字节序
*				pbuf：发送数据buffer
*				tlen: 发送数据长度
*           
*
* Returns    :   函数调用返回结果：成功 返回 socket id > 0  失败返回 < 0
				

* Note(s)    :  函数通过直接通过端口发送 不会创建线程，发送完成之后释放资源
				
**/

S32  ks_lwip_tcp_poll_send(U32 dstip, U16 dstport, U8*pbuf,U32 tlen);



/**
*                                      ks_lwip_udp_send  
*
* Description:  udp   发送数据接口

* Arguments  :  
*				dstip：发送目的IP     本地字节序
*				dstport：发送目的端口号，本地字节序
*				pbuf：发送数据buffer
*				tlen: 发送数据长度
*           
*
* Returns    :   函数调用返回结果：成功 返回 socket id > 0  失败返回 < 0
				

* Note(s)    :  函数通过直接通过端口发送 不会创建线程，发送完成之后释放资源
				
**/

S32  ks_lwip_udp_poll_send(U32 dstip, U16 dstport, U8*pbuf,U32 tlen);


/**
*                                      ks_lwip_upate_eth_netcfg  
*
* Description:  更新网络初始化配置IP
* Arguments  :  
*				 
*
* Returns    :  
				
* Note(s)    :  
				
**/


S32 ks_lwip_upate_eth_netcfg(LwipNetCfg* param);


/**
*                                      ks_lwip_init  
*
* Description:  LWIP 模块初始化       创建task    初始化IP
* Arguments  :  
*				 
*
* Returns    :  
				
* Note(s)    :  
				
**/

void ks_lwip_init(LwipNetCfg*  pNetCfg);

#endif

