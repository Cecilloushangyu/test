#include <stdint.h>
#include "ks_mem.h"
#include "ks_eth.h"
#include "eth_controller.h"


void ks_driver_eth_init(U8* mac)
{
	EthInit(mac);

}

U8* ks_driver_eth_get_mac(){

	return EthGetMac();
}


S32 ks_driver_eth_add_data_recv_callback(EthDataCbk cb_func)
{
	return EthData_AddRecvCallBack(cb_func);
}

S32 ks_driver_eth_add_status_change_callback(EthStatusCbk cb_func)
{
	return EthStatus_AddChangeCallBack(cb_func);
}

S32 ks_driver_eth_send_buffer(U8 *p_data, U32 len)
{
	return EthData_Send(p_data,len);
}

S32 ks_driver_eth_get_send_pbuffer(eth_pkt_buf** pbuffer)
{

	return EthData_GetSendBuffer(pbuffer);

}

S32 ks_driver_eth_flag_send_task()
{

	return EthData_FlagSendTask();

}




