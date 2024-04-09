#ifndef PLATFORM_TASK_DEFINE_H_
#define PLATFORM_TASK_DEFINE_H_



// priority set 8~23 for user
#define _THREAD_PRI_IPC_RECEIVE			3


#define _THREAD_PRI_SHELL				8
#define	_THREAD_PRI_CAN_ENTRY			7
#define	_THREAD_PRI_UART_RECEIVE		9
#define _THREAD_PRI_ETH_DATA         	12

#define _THREAD_PRI_LWIP_MAIN          13

#define _THREAD_PRI_LWIP_SERVER        14
#define _THREAD_PRI_LWIP_CLIENT        15

#define _THREAD_PRI_MMCSD_DETECT      	16

#define _THREAD_PRI_UART_PRINT			9
#define _THREAD_PRI_CAN_PRINT			29
#define _THREAD_PRI_ETH_STATUS      	30


#define _THREAD_PRI_STARTUP				30


//threadx  系统没有空闲任务　创建统计task tick count  
#define _THREAD_PRI_IDLE			31


#endif /* PLATFORM_TASK_DEFINE_H_ */
