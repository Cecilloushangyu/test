#include "string.h"
#include "lwip/opt.h"
#include "lwip/sys.h"
#include "arch/sys_arch.h"
#include "ks_datatypes.h"
#include "ks_os.h"
#include "ks_printf.h"
#include "ks_exception.h"

uint32_t osGetTimeTick(void)
{
	uint32_t curtimer =ks_os_chrono_current_time()/1000000;
	return curtimer;
}

void sys_init(void)
{

}

err_t sys_sem_new(sys_sem_t *sem, u8_t count){
	OSHandle handle;
	static int sem_new;
	char name[20];
	memset(name,0,sizeof(name));
	sprintf(name,"lwip_sem_%d",sem_new);
	sem_new++;
	int ret =ks_os_sem_create (&handle,name,count);
	if(ret!=0) {
		kprintf("sys_sem_new err %d    \r\n",ret);
		ks_exception_assert(0);
	}
	*sem =(sys_sem_t ) handle;
	//kprintf("sys_sem_new *sem %x   \r\n",*sem);
	return ERR_OK;

}


void sys_sem_free(sys_sem_t *sem)
{
	ks_os_sem_delete ((OSHandle)*sem);
	//kprintf("sys_sem_free *sem %x   \r\n",*sem);

}

void sys_sem_signal(sys_sem_t*sem)
{
	//kprintf("sys_sem_signal *sem %x   \r\n",*sem);
    ks_os_sem_post((OSHandle)*sem);
}

u32_t sys_arch_sem_wait(sys_sem_t*sem, u32_t timeout)
{
    u32_t t=osGetTimeTick();
	//kprintf("sys_arch_sem_wait *sem %x  timeout %d \r\n",*sem,timeout);
	if(timeout == 0 ){
		ks_os_sem_pend((OSHandle)*sem,KS_OS_WAIT_FOREVER);
	}
	else{
		ks_os_sem_pend((OSHandle)*sem,timeout);
	}
    t=osGetTimeTick()-t;
    if(t>timeout)
    	t=SYS_ARCH_TIMEOUT;
    return t;

}


err_t sys_mbox_new(sys_mbox_t *mbox, int size)
{
	OSHandle queue;
	static int mbox_new;
	char name[20];
	memset(name,0,sizeof(name));
	sprintf(name,"lwip_mbox_%d",mbox_new);
	mbox_new++;
	int ret = ks_os_queue_create(&queue,name,size,sizeof(void*));
	if(ret!=0) ks_exception_assert(0);
	*mbox = queue;
	//kprintf("sys_mbox_new *mbox %x   \r\n",*mbox);
	return ERR_OK;

}

void sys_mbox_free(sys_mbox_t *mbox){

    ks_os_queue_delete(*mbox);
}


void sys_mbox_post(sys_mbox_t *mbox, void *msg){
	ubase_t ptr = (ubase_t) msg;
	//kprintf("sys_mbox_post *mbox %x  msg %x \r\n",*mbox,msg);
	int ret =ks_os_queue_send ((OSHandle)*mbox,&ptr,KS_OS_WAIT_FOREVER);
	if(ret!=0){
		ks_os_printf(0,"sys_mbox_post ERR %d +++++ \r\n",ret);
	}
}


err_t sys_mbox_trypost(sys_mbox_t *mbox, void *msg){
	ubase_t ptr = (ubase_t)msg;
	//kprintf("sys_mbox_trypost *mbox %x  msg %x \r\n",*mbox,msg);
	return ks_os_queue_send ((OSHandle)*mbox,&ptr,KS_OS_NO_WAIT);
}


u32_t sys_arch_mbox_fetch(sys_mbox_t *mbox, void **msg, u32_t timeout){
	u32_t t=osGetTimeTick();
	int ret ;
	ubase_t ptr ;

	if(timeout== 0 ){
		ret = ks_os_queue_receive((OSHandle)*mbox,&ptr,KS_OS_WAIT_FOREVER);
	}else{
		ret = ks_os_queue_receive((OSHandle)*mbox,&ptr,timeout);
	}
	//kprintf("sys_arch_mbox_fetch mbox %x  msg %x \r\n",mbox,ptr);

	*msg = (void*) ptr;

	if(ret== 0){

		t=osGetTimeTick()-t;
		return t;
	}else{
		//osPrintf("sys_arch_mbox_fetch err %d  %d\r\n",ret,timeout);
	}
	return SYS_ARCH_TIMEOUT;

}

u32_t sys_arch_mbox_tryfetch(sys_mbox_t *mbox, void **msg){

	ubase_t ptr ;

	int ret =  ks_os_queue_receive((OSHandle)*mbox,&ptr,KS_OS_NO_WAIT);
	//kprintf("sys_arch_mbox_tryfetch mbox %x  msg %x \r\n",mbox,ptr);
	*msg = (void *) ptr;
	
	if(ret == 0 ){
		return 0;
	}
	else{
		//ks_os_printf(0,"sys_arch_mbox_tryfetch osMailBoxAccept ERR %d +++++ \r\n",ret);
        return SYS_MBOX_EMPTY;
    } 
}




