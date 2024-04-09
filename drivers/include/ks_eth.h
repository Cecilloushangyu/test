#ifndef INTERFACE_INCLUDE_KS_ETH_H_
#define INTERFACE_INCLUDE_KS_ETH_H_
#include <stdint.h>
#include "ks_datatypes.h"

#ifdef __cplusplus
extern "C" {
#endif


typedef   struct  _tag_ether_header
{
	U8   ether_dhost[6];   //目的MAC地址
	U8   ether_shost[6];   //源MAC地址
	U16  ether_type;        //帧类型
}ether_header;



typedef struct _tag_eth_pkt_buf
{
	U16  length;
	U16  tsize;
	U8  buf[0];
	
} eth_pkt_buf;


typedef void (*EthStatusCbk)(     U32 linkflag, U32 linkspeed);

typedef void (*EthDataCbk)( U8 * recvbuffer,U32 len);

/**
 * @brief  eth 初始化网口
 * @param  mac mac 地址
 */
void ks_driver_eth_init(U8* mac);
/**
 * @brief  eth　获取mac地址
 * @retval 返回 mac 地址
 */
U8* ks_driver_eth_get_mac();
/**
 * @brief 注册ETH数据接收回调接口
 * @param  cb_func　接收回调函数
 */
S32 ks_driver_eth_add_data_recv_callback(EthDataCbk cb_func);
/**
 * @brief 注册ETH状态变化回调接口
 * @param  cb_func　状态变化回调函数
 */

S32 ks_driver_eth_add_status_change_callback(EthStatusCbk cb_func);

/**
 * @brief 注册ETH　发送数据接口
 * @param  cb_func　状态变化回调函数
 */

S32 ks_driver_eth_send_buffer( U8 *p_data, U32 len);

//发送队列中获取发送buffer,减少拷贝次数，提高发送效率
S32 ks_driver_eth_get_send_pbuffer(eth_pkt_buf** pbuffer);
//flag异步通知eth发送任务
S32 ks_driver_eth_flag_send_task();

#ifdef __cplusplus
}
#endif

#endif /* INTERFACE_INCLUDE_AP_UAH_ */
