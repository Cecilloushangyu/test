
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  demo3_semaphore.c
 *
 * @brief  os 测试例程: 信号量
 */


#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "os_demo.h"

#define DEMO_FINISHED_1 1
#define DEMO_FINISHED_2 2
#define DEMO_FINISHED_3 4

static OSHandle flag_handle_sem_demo_finish;
static OSHandle sem_handle_sem_demo;

#define  STACK_SIZE_SEM_DEMO          512
static S32 thread_stack_sem_demo_task_producer[STACK_SIZE_SEM_DEMO];
static OSHandle thread_handle_sem_demo_task_producer;

static S32 thread_stack_sem_demo_task_consumer1[STACK_SIZE_SEM_DEMO];
static OSHandle thread_handle_sem_demo_task_consumer1;

static S32 thread_stack_sem_demo_task_consumer2[STACK_SIZE_SEM_DEMO];
static OSHandle thread_handle_sem_demo_task_consumer2;

void SemaphoreDemoConsumer(void *p_arg)
{
    char print_buffer[128];
    S32 sleep_interval = 15;
    U32 current_sem_count;

    ks_os_thread_vfp_enable();

    S32 consumer_id = (S32) p_arg;
    for (S32 i = 0; i < 5; ++i)
    {
        // 等待生产完成
        ks_os_sem_pend(sem_handle_sem_demo, KS_OS_WAIT_FOREVER);
        // 查询当前信号量计数
        current_sem_count = ks_os_sem_peek(sem_handle_sem_demo);
        sprintf(print_buffer, "Consumer #%d, consume #%d, pending count %d\r\n",
                consumer_id, i + 1, current_sem_count);
        ks_driver_uart_send_string(0, print_buffer);
        // 睡眠间隔时间，模拟消费者消费时间
        ks_os_thread_sleep(sleep_interval);
    }

    // 发送线程已完成标记
    ks_os_flag_set_bits(flag_handle_sem_demo_finish,
                        consumer_id == 1 ? DEMO_FINISHED_2 : DEMO_FINISHED_3, SET_FLAG_MODE_OR);
}

void SemaphoreDemoTaskProducer(void *p_arg)
{
    char print_buffer[128];
    // 生产者生产时间间隔
    S32 sleep_intervals[] = {
        1, 2, 3, 4, 5, 6, 7, 8, 9, 10
    };

    // 使能浮点运算
    ks_os_thread_vfp_enable();

    for (S32 i = 0; i < 10; ++i)
    {
        // 睡眠间隔时间，模拟生产者生产时间
        ks_os_thread_sleep(sleep_intervals[i]);
        // 模拟生产完成
        sprintf(print_buffer, "Generate #%d\r\n", i + 1);
        ks_driver_uart_send_string(0, print_buffer);
        // 通知生产完成
        ks_os_sem_post(sem_handle_sem_demo);
    }

    // 发送线程已完成标记
    ks_os_flag_set_bits(flag_handle_sem_demo_finish, DEMO_FINISHED_1, SET_FLAG_MODE_OR);
}

void SemaphoreDemo(void)
{
    U32 ret;
    ks_driver_uart_send_string(0, "-------------------------------\r\n");
    ks_driver_uart_send_string(0, "----Semaphore Demo begins------\r\n");

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_sem_demo_finish, "flag_sem_demo_finish");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建信号量
    ret = ks_os_sem_create(&sem_handle_sem_demo, "sem_sem_demo",0);
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating semaphore!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_sem_demo_task_producer,
                              "sem_demo_task_producer",
                              SemaphoreDemoTaskProducer,
                              0,
                              20,
                              thread_stack_sem_demo_task_producer,
                              sizeof(thread_stack_sem_demo_task_producer),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }
    ret = ks_os_thread_create(&thread_handle_sem_demo_task_consumer1,
                              "sem_demo_task_consumer1",
                              SemaphoreDemoConsumer,
                              (void *) 1,
                              21,
                              thread_stack_sem_demo_task_consumer1,
                              sizeof(thread_stack_sem_demo_task_consumer1),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }
    ret = ks_os_thread_create(&thread_handle_sem_demo_task_consumer2,
                              "sem_demo_task_consumer2",
                              SemaphoreDemoConsumer,
                              (void *) 2,
                              21,
                              thread_stack_sem_demo_task_consumer2,
                              sizeof(thread_stack_sem_demo_task_consumer2),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    // 等待标识量（Demo线程完成）
    ks_os_flag_wait(flag_handle_sem_demo_finish,
                    DEMO_FINISHED_1 | DEMO_FINISHED_2 | DEMO_FINISHED_3,
                    WAIT_FLAG_MODE_AND_CLEAR, KS_OS_WAIT_FOREVER);

    ks_driver_uart_send_string(0, "----Semaphore Demo ends--------\r\n");
}

