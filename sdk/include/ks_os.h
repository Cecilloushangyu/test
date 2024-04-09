#pragma once
#include "ks_datatypes.h"


#ifdef __cplusplus
extern "C" {
#endif

#define	OSHANDLE_FAIL_NAME_LENGTH		1
#define	OSHANDLE_FAIL_NAME_OCCUPY		2
#define	OSHANDLE_FAIL_EXCEED_LIMIT		3
#define	OSHANDLE_FAIL_CREATE			4
#define OSHANDLE_SUCCESS				0

#define TICK_PER_SECOND  100
typedef U32						OSHandle;


// threads API
typedef void (*OSThreadEntry) (void *arg);
U32 ks_os_thread_create(OSHandle *thread_handle, char *thread_name, OSThreadEntry thread_entry, void *arg, S32 priority, void *stack_start, U32 stack_size, U32 time_slice, U32 auto_start);
void ks_os_thread_vfp_enable(void);
void ks_os_thread_sleep(S32 os_tick);
void ks_os_thread_sleep_msec(U32 msec);
void ks_os_thread_suspend(OSHandle thread_handle);
void ks_os_thread_resume(OSHandle thread_handle);
char * ks_os_thread_get_thread_name(OSHandle  thread_handle);
OSHandle  ks_os_thread_get_thread_handle(char *thread_name);
OSHandle  ks_os_thread_get_current_thread_handle();

//task stack  api 
void*  ks_os_thread_get_current_stack(OSHandle  thread_handle);
void*  ks_os_thread_get_stack_highest_ptr(OSHandle  thread_handle);
void*  ks_os_thread_get_stack_start(OSHandle  thread_handle);
void*  ks_os_thread_get_stack_end(OSHandle  thread_handle);

// flag API
#define KS_OS_WAIT_FOREVER			0xFFFFFFFF
#define KS_OS_NO_WAIT				0

typedef enum {WAIT_FLAG_MODE_OR = 0, WAIT_FLAG_MODE_AND, WAIT_FLAG_MODE_OR_CLEAR, WAIT_FLAG_MODE_AND_CLEAR} WaitFlagMode;
typedef enum {SET_FLAG_MODE_OR = 0, SET_FLAG_MODE_AND} SetFlagMode;
U32 ks_os_flag_create(OSHandle *flag_handle, char *flag_name);
U32 ks_os_flag_wait(OSHandle flag_handle, U32 value, WaitFlagMode mode, U32 timeout);
void ks_os_flag_set_bits(OSHandle flag_handle, U32 value, SetFlagMode mode);
U32 ks_os_flag_delete(OSHandle flag_handle);

// sem API
U32 ks_os_sem_create(OSHandle *sem_handle, char *sem_name, S32 initial_count);
void ks_os_sem_post(OSHandle sem_handle);
U32 ks_os_sem_pend(OSHandle sem_handle, U32 timeout);
U32 ks_os_sem_peek(OSHandle sem_handle);
U32 ks_os_sem_delete(OSHandle sem_handle);

// mail box API
U32 ks_os_mbx_create(OSHandle *mbx_handle, char *mbx_name, U32 *p_queue, U32 queue_size);
U32 ks_os_mbx_post(OSHandle mbx_handle, void *p_mbx_data);
U32 ks_os_mbx_pend(OSHandle mbx_handle, void **p_mbx_data, U32 timeout);
U32 ks_os_mbx_delete(OSHandle mbx_handle);


//  msg queue API
U32 ks_os_queue_create(OSHandle *queue,char *queue_name, U32 length, U32 element_size);
U32 ks_os_queue_send(OSHandle queue, void *msg, U32 timeout_ms);
U32 ks_os_queue_receive(OSHandle queue, void *msg, U32 timeout_ms);
U32 ks_os_get_queue_size(OSHandle queue);
U32 ks_os_queue_delete(OSHandle queue);

// mutex API
U32 ks_os_mutex_create(OSHandle *mutex_handle, char *mutex_name);
U32 ks_os_mutex_enter(OSHandle mutex_handle);
U32 ks_os_mutex_leave(OSHandle mutex_handle);
U32 ks_os_mutex_delete(OSHandle mutex_handle);

// irq API

typedef void (*IRQEntry)(void* arg);
void ks_os_irq_create(int irq_vec_num, IRQEntry handler, void* arg);
void ks_os_irq_enable(S32 irq_vec_num);
void ks_os_irq_disable(S32 irq_vec_num);
void ks_os_irq_map_target(S32 irq_vec_num, S32 cpu_target);
void ks_os_irq_vfp_enable(S32 irq_vec_num);
void ks_os_irq_mask_all(void);
void ks_os_irq_unmask_all(void);
U32 ks_os_irq_get_counter(S32 irq_vec_num);

// timer API
typedef void (*TimerEntry)(void* arg);
U32 ks_os_timer_coarse_create(OSHandle *timer_handle, char *timer_name, TimerEntry timer_entry, void* arg, U32 initial_ms, U32 interval_ms, U32 auto_activate);
U32 ks_os_timer_coarse_activate(OSHandle timer_handle);
U32 ks_os_timer_coarse_deactivate(OSHandle timer_handle);
U32 ks_os_timer_coarse_change(OSHandle timer_handle, TimerEntry timer_entry, void* arg, U32 initial_ms, U32 interval_ms);
U32 ks_os_timer_coarse_info_get(OSHandle timer_handle, char **timer_name, U32 *is_active, U32 *remaining_ms, U32 *interval_ms);
U32 ks_os_timer_accurate_create(TimerEntry timer_entry, void* arg, U32 initial_us, U32 interval_us, U32 auto_activate);
U32 ks_os_timer_accurate_activate();
U32 ks_os_timer_accurate_deactivate();
U32 ks_os_timer_accurate_change(TimerEntry timer_entry, void* arg, U32 initial_us, U32 interval_us);
U32 ks_os_timer_accurate_info_get(U32 *is_active, DOUBLE *remaining_us, U32 *interval_us);

// chrono API
U64 ks_os_chrono_elapsed_time(void);
U64 ks_os_chrono_current_time(void);
void ks_os_chrono_set_time(U64 time_ns);

// delay API poll delay
void ks_os_poll_delay_msec(U32 msec);
void ks_os_poll_delay_usec(U32 usec);

// load API
U64 ks_os_load_get_thread_time(OSHandle thread_handle);
U64 ks_os_load_get_irq_time(void);
U64 ks_os_load_get_idle_time(void);

// system clock
DOUBLE ks_os_get_sys_clock(void);
DOUBLE ks_os_get_apb_clock(void);
DOUBLE ks_os_get_ahb_clock(void);

// timing
U64 ks_os_get_free_time(void);

// memory
U32 ks_os_get_heap_top_unused_memory();

U32 ks_os_tick_to_msec(U32 tick);
U32 ks_os_msec_to_tick(U32 msec);



#ifdef __cplusplus
}
#endif

