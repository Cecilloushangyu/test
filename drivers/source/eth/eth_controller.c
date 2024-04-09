

#include "dlist.h"
#include "string.h"
#include "ks_taskdef.h"
#include "ks_os.h"

#include "synop_ethmac.h"
#include "eth_controller.h"
#include "eth_low_level_driver.h"
#include "ks_shell.h"
#include "queue_array.h"
#include "ks_printf.h"
#include "ipaddr_utils.h"
#include "ks_cache.h"


#define ETH_DEFAULT_MAC  "CC:CC:CC:CC:CC:CC"
static const uint8_t default_mac[6] = { 0xcc, 0xcc, 0xcc, 0xcc, 0xcc, 0xcc };



typedef struct  EthDataRecvListCb
{
	struct list_node  cb_list;
	EthDataCbk cbfunc;
	int is_used;
}EthDataRecvListCb;


typedef struct  EthStatusListCb
{
	struct list_node  cb_list;
	EthStatusCbk cbfunc;
	int is_used;
}EthStatusListCb;

EthDataRecvListCb s_ethrecvcb[4];
EthStatusListCb s_ethstatuscb[4];

// eth  entry
#define STACK_SIZE_ETH_ENTRY		

struct net_device g_netdevice ;

OSHandle g_EthStatusFlag;
OSHandle g_EthDataFlag;
OSHandle g_EthRxDataFlag;

OSHandle  g_EthTimer; 


OSHandle thread_eth_tx_data_entry;
int thread_stack_eth_tx_data_entry[1024];


OSHandle thread_eth_rx_data_entry;
int thread_stack_eth_rx_data_entry[1024];

OSHandle thread_eth_status_entry;
int thread_stack_eth_status_entry[512];

struct list_node  g_EthDataRecvCallBackList;	
struct list_node  g_EthStatusChangeCallBackList;	

static OSHandle mutex_handle_eth_tx;


int init_flag_eth = 0;


#define QUEUE_ARRAY_LENTH 8
uint8_t g_eth_tx_queuearray_buffer[sizeof(queue_array)+DMA_BUF_SIZE*QUEUE_ARRAY_LENTH] __attribute__((aligned(8)));
static queue_array* g_tx_queuearray;

void EthNoticeLink(int linkflag)
{
    kprintf("EthNoticeLink  %d \r\n",linkflag);
	ks_os_flag_set_bits(g_EthStatusFlag, ETH_LINK_STATUS, SET_FLAG_MODE_OR);

}

void EthTimerCallback(void* arg)
{
	ks_os_flag_set_bits(g_EthStatusFlag, ETH_TICK_STATUS, SET_FLAG_MODE_OR);
}

void EthNoticeRecv(int args)
{
	ks_os_flag_set_bits(g_EthRxDataFlag, ETH_RX_DATA, SET_FLAG_MODE_OR);
}

void EthNoticeXmit(int args)
{
	ks_os_flag_set_bits(g_EthDataFlag, ETH_TX_DATA, SET_FLAG_MODE_OR);
}

void EthDataRecvCb( uint8_t * recvbuffer,int len)
{
	uint8_t * nocachebuffer;

	if(eth_isready(&g_netdevice))
	{
		EthDataRecvListCb * pcallback;
		
		list_for_every_entry( &g_EthDataRecvCallBackList, pcallback, EthDataRecvListCb, cb_list )
		{	
			if(pcallback->is_used&&pcallback->cbfunc!=NULL){
				ks_cpu_dcache_invalidate(recvbuffer, len);
				//nocachebuffer = recvbuffer+MMU_NO_CACHE_OFFSET;
				pcallback->cbfunc(recvbuffer,len);
				//pcallback->cbfunc(recvbuffer,len);
				//ks_os_printf(0,"rx:");
				//ks_os_printf_hex(0,nocachebuffer, len);
			}						
		}
		
   	}
}

int EthDataTxBufferCb( uint8_t * txbuffer,int bufferlen)
{	
	int len;
	len = 0;
	int ret ;
	uint8_t * nocachebuffer;
	eth_pkt_buf* pbuffer;
	if(eth_isready(&g_netdevice))
	{
		ks_os_mutex_enter(mutex_handle_eth_tx);
		ret = queue_array_pop_buffer(g_tx_queuearray,NULL,0,(void**)&pbuffer);
		if(ret == 0){

			len = pbuffer->length;
			if( len> 0 && len< DMA_BUF_SIZE){
				memcpy(txbuffer,pbuffer->buf,len);
				ks_cpu_dcache_clean( txbuffer, len);
			}
		
		}
		ks_os_mutex_leave(mutex_handle_eth_tx);
		if(ret == 0){
			//ks_os_printf(0,"tx:");
			//ks_os_printf_hex(0,nocachebuffer, len);
			return len ;
		}

   	}
	return 0 ;
}

int EthStatusIsLink()
{
	return g_netdevice.linkflag;
}



int EthData_GetSendBuffer(eth_pkt_buf** pbuffer)
{
	int ret ;
	ks_os_mutex_enter(mutex_handle_eth_tx);
	ret = queue_array_push_buffer(g_tx_queuearray,NULL,0,(void**)pbuffer);
	ks_os_mutex_leave(mutex_handle_eth_tx);
	return ret ;

}

int EthData_FlagSendTask()
{
	ks_os_flag_set_bits(g_EthDataFlag, ETH_TX_DATA, SET_FLAG_MODE_OR);
	return 0 ;
}


int EthData_Send(uint8_t *p_data, uint32_t len)
{

	int ret ;
	eth_pkt_buf* pbuffer;
	ret = EthData_GetSendBuffer(&pbuffer);
	if(ret==0){
		if( len> 0 && len< DMA_BUF_SIZE){
			memcpy(pbuffer->buf,p_data,len);
			pbuffer->length = len;
			ks_os_flag_set_bits(g_EthDataFlag, ETH_TX_DATA, SET_FLAG_MODE_OR);
		}else{
			return -4;
		}
	}
	
	return ret ;
}



int EthData_AddRecvCallBack(EthDataCbk cb_func)
{

	EthDataRecvListCb* precvcb = NULL;

	int count = sizeof(s_ethrecvcb)/sizeof(s_ethrecvcb[0]);
	for(int i = 0;i<count;i++)
	{
		if(s_ethrecvcb[i].is_used==0)
		{
			precvcb = &s_ethrecvcb[i];
			precvcb->is_used=1;
			break;
		}
	}

	if(precvcb!=NULL){
		precvcb->cbfunc=cb_func;
		list_add_tail( &(g_EthDataRecvCallBackList), &(precvcb->cb_list) );
		return 0 ;
	}else{
		return -1;
	}

}


int EthStatus_AddChangeCallBack(EthStatusCbk cb_func)
{

	EthStatusListCb* precvcb = NULL;
	int count = sizeof(s_ethstatuscb)/sizeof(s_ethstatuscb[0]);
	for(int i = 0;i<count;i++)
	{
		if(s_ethstatuscb[i].is_used==0)
		{
			precvcb = &s_ethstatuscb[i];
			precvcb->is_used=1;
			break;
		}
	}

	if(precvcb!=NULL){
		precvcb->cbfunc=cb_func;
		list_add_tail( &(g_EthStatusChangeCallBackList), &(precvcb->cb_list) );
		if(g_netdevice.linkflag!=0){
			cb_func(g_netdevice.linkflag,g_netdevice.linkspeed);
		}
		return 0 ;
	}else{
		return -1;
	}

}




void EthRxDataTask(void *p_arg)
{
    int i, result;
    uint32_t curFlag ;
    int iret;
	
	g_netdevice.cb_recvlist = EthDataRecvCb;

	while (1)
	{
	
	   curFlag = ks_os_flag_wait(g_EthRxDataFlag, ETH_RX_DATA, WAIT_FLAG_MODE_OR_CLEAR,KS_OS_WAIT_FOREVER);

        if (curFlag & ETH_RX_DATA)
		{
			eth_rx(&g_netdevice);
	    }
   }
}


void EthTxDataTask(void *p_arg)
{
    int i, result;
    uint32_t curFlag ;
    int iret;
	
	g_netdevice.cb_txbuffer = EthDataTxBufferCb;

	queue_array_create((queue_array *)g_eth_tx_queuearray_buffer,"eth_tx_queue",4,DMA_BUF_SIZE,QUEUE_ARRAY_LENTH);

	g_tx_queuearray = (queue_array *) g_eth_tx_queuearray_buffer;


	while (1)
	{
	
	   curFlag = ks_os_flag_wait(g_EthDataFlag, ETH_TX_DATA, WAIT_FLAG_MODE_OR_CLEAR,KS_OS_WAIT_FOREVER);

	    if (curFlag & ETH_TX_DATA)
		{
			 eth_tx(&g_netdevice);
		}
   }
}


void EthStatusTask(void *p_arg)
{
	int i, result;
    uint16_t status;
    uint32_t Status,Interrupt;
    int iret;
    uint32_t curFlag ;
	

    iret = ks_os_timer_coarse_create(&g_EthTimer,"eth status timer",EthTimerCallback,&g_netdevice, 100, 1000, 1);

    if(iret!= 0 ){
        kprintf("TimerCreate failuer %d \r\n",iret);
    }

	while (1)
	{
	
		curFlag = ks_os_flag_wait(g_EthStatusFlag, ETH_TICK_STATUS|ETH_LINK_STATUS, WAIT_FLAG_MODE_OR_CLEAR,KS_OS_WAIT_FOREVER);
	    if (curFlag & ETH_TICK_STATUS)
		{
			if(g_netdevice.flag & ETH_FLAG_INIT_OK)
			{
				EthPhy_UpdateStatus(&g_netdevice);
			}
		}

      	if (curFlag & ETH_LINK_STATUS)
		{
			EthStatusListCb * pcallback;
			list_for_every_entry( &g_EthStatusChangeCallBackList, pcallback, EthStatusListCb, cb_list )
			{	
				if(pcallback->is_used&&pcallback->cbfunc!=NULL){
					pcallback->cbfunc(g_netdevice.linkflag,g_netdevice.linkspeed);
				}						
			}

		}
	}
}


uint8_t* EthGetMac()
{
	return g_netdevice.mac;
}


void EthInit(uint8_t* mac)
{

	if(init_flag_eth == 0){

		EthPort_Init();

		eth_init(&g_netdevice);
	
		ks_os_mutex_create(&mutex_handle_eth_tx, "mutex_eth_tx");

		ks_os_flag_create(&g_EthStatusFlag, "eth status");

	    ks_os_flag_create(&g_EthRxDataFlag, "eth rx data");
		
		ks_os_flag_create(&g_EthDataFlag, "eth data");

	    list_initialize(&g_EthDataRecvCallBackList);
		
		list_initialize(&g_EthStatusChangeCallBackList);

		//eth_shell_cmd_init();

		ks_os_thread_create(&thread_eth_status_entry,			//	thread_handle
	    			 "eth status task",					//	thread_name
	    			 EthStatusTask,						//	thread_entry
	    			 0,									//	arg
	    			 _THREAD_PRI_ETH_STATUS,					 //	priority
	    			 thread_stack_eth_status_entry,			//	stack_start
	    			 sizeof(thread_stack_eth_status_entry),	//	stack_size
					 0,
					 1
					 );

		 ks_os_thread_create(&thread_eth_rx_data_entry,			//	thread_handle
					"eth rx task",					//	thread_name
					EthRxDataTask,						//	thread_entry
					0,									//	arg
					6,				//	priority
					thread_stack_eth_rx_data_entry,			//	stack_start
					sizeof(thread_stack_eth_rx_data_entry),	//	stack_size
					0,
					1
					);
		
		  ks_os_thread_create(&thread_eth_tx_data_entry,			 //  thread_handle
					"eth tx task",					 //  thread_name
					EthTxDataTask,						 //  thread_entry
					0,								 //  arg
					21, 			 //  priority
					thread_stack_eth_tx_data_entry,			 //  stack_start
					sizeof(thread_stack_eth_tx_data_entry),	 //  stack_size
					0,
					1
					);

		if(mac==NULL)
		{
			mac_atoh(ETH_DEFAULT_MAC, g_netdevice.mac);
		}
		else
		{
			mac_atoh((char *)mac, g_netdevice.mac);
		}

		init_flag_eth = 1;

	}

}




