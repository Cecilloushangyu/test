
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  demo5_mutex.c
 *
 * @brief  os 测试例程: 互斥量
 */


#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "os_demo.h"

#define DEMO_FINISHED_1 1
#define DEMO_FINISHED_2 2
#define DEMO_FINISHED_3 4
#define DEMO_FINISHED_4 8
#define DEMO_QUEUE_NON_EMPTY 1
#define DEMO_QUEUE_EXIT_1 (1<<30)
#define DEMO_QUEUE_EXIT_2 (1<<31)

static OSHandle flag_handle_mutex_demo_finish;
static OSHandle flag_handle_mutex_demo_queue_non_empty;

#define  STACK_SIZE_MUTEX_DEMO          512
static S32 thread_stack_mutex_demo_task_1[STACK_SIZE_MUTEX_DEMO];
static OSHandle thread_handle_mutex_demo_task_1;

static S32 thread_stack_mutex_demo_task_2[STACK_SIZE_MUTEX_DEMO];
static OSHandle thread_handle_mutex_demo_task_2;

static S32 thread_stack_mutex_demo_task_3[STACK_SIZE_MUTEX_DEMO];
static OSHandle thread_handle_mutex_demo_task_3;

static S32 thread_stack_mutex_demo_task_4[STACK_SIZE_MUTEX_DEMO];
static OSHandle thread_handle_mutex_demo_task_4;

#define QUEUE_BUF_SIZE            1024

typedef struct
{
    U8 queue_buf[QUEUE_BUF_SIZE];
    S32 tx_read;
    S32 tx_write;
    S32 tx_valid;
} Queue, *pQueue;

Queue queue;

static OSHandle mutex_handle_mutex_demo;

S32 en_queue_buffer(pQueue p_queue, const U8 *p_data, S32 len)
{
    // 锁定互斥量
    ks_os_mutex_enter(mutex_handle_mutex_demo);
    S32 residual = QUEUE_BUF_SIZE - p_queue->tx_valid;
    if (len > residual)
        len = residual;
    S32 t = p_queue->tx_write + len;
    if (t > QUEUE_BUF_SIZE)
    {
        S32 temp = QUEUE_BUF_SIZE - p_queue->tx_write;
        memcpy(&p_queue->queue_buf[p_queue->tx_write], p_data, temp);
        memcpy(p_queue->queue_buf, p_data + temp, len - temp);
        p_queue->tx_write = len - temp;
    }
    else
    {
        memcpy(&p_queue->queue_buf[p_queue->tx_write], p_data, len);
        p_queue->tx_write += len;
    }
    p_queue->tx_valid += len;
    // 解锁互斥量
    ks_os_mutex_leave(mutex_handle_mutex_demo);
    return len;
}

S32 de_queue_buffer(pQueue p_queue, U8 *p_buffer, S32 len)
{
    // 锁定互斥量
    ks_os_mutex_enter(mutex_handle_mutex_demo);
    if (len > p_queue->tx_valid)
        len = p_queue->tx_valid;
    S32 t = p_queue->tx_read + len;
    if (t > QUEUE_BUF_SIZE)
    {
        S32 temp = QUEUE_BUF_SIZE - p_queue->tx_read;
        memcpy(p_buffer, &p_queue->queue_buf[p_queue->tx_read], temp);
        memcpy(p_buffer + temp, p_queue->queue_buf, len - temp);
        p_queue->tx_read = len - temp;
    }
    else
    {
        memcpy(p_buffer, &p_queue->queue_buf[p_queue->tx_read], len);
        p_queue->tx_read += len;
    }
    p_queue->tx_valid -= len;
    // 解锁互斥量
    ks_os_mutex_leave(mutex_handle_mutex_demo);
    return len;
}

void MutexDemoConsumer(void *p_arg)
{
    U8 buffer[128];
    char print_buffer[128];
    S32 thread_id = (S32)p_arg;
    S32 len;
    U32 flag;

    ks_os_thread_vfp_enable();

    U32 flag_exit = (thread_id==1?DEMO_QUEUE_EXIT_1:DEMO_QUEUE_EXIT_2);
    while(1)
    {
        flag = ks_os_flag_wait(flag_handle_mutex_demo_queue_non_empty, DEMO_QUEUE_NON_EMPTY|flag_exit, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
        if (flag & DEMO_QUEUE_NON_EMPTY)
        {
            len = de_queue_buffer(&queue, (U8*)buffer, 128);
            if (len)
            {
                sprintf(print_buffer, "Consumer %d Got -- ", thread_id);
                ks_driver_uart_send_string(0, print_buffer);
                ks_driver_uart_send_buffer(0, buffer, len);
                ks_driver_uart_send_string(0, "\r\n");
                // 睡眠间隔时间，模拟消费者消费时间
                ks_os_thread_sleep(2);
            }
        }
        if (flag & flag_exit)
        {
            break;
        }
    }

    // 发送线程已完成标记
    ks_os_flag_set_bits(flag_handle_mutex_demo_finish, (thread_id==1?DEMO_FINISHED_3:DEMO_FINISHED_4), SET_FLAG_MODE_OR);
}

void MutexDemoGenerator(void *p_arg)
{
    char buffer[128];
    char print_buffer[128];
    S32 thread_id = (S32) p_arg;
    const char *contents[5] = {
        "Generator %d, Content1; ",
        "Generator %d, Content2; ",
        "Generator %d, Content3; ",
        "Generator %d, Content4; ",
        "Generator %d, Content5; ",
    };

    // 使能浮点运算
    ks_os_thread_vfp_enable();

    for (S32 i = 0; i < 5; ++i)
    {
        // 睡眠
        ks_os_thread_sleep(1);
        // 入队
        sprintf(buffer, contents[i], thread_id);
        en_queue_buffer(&queue, (U8 *) buffer, (S32) strlen(contents[i]));
        sprintf(print_buffer, "Thread %d, Enqueue #%d\r\n", thread_id, i + 1);
        ks_driver_uart_send_string(0, print_buffer);
        ks_os_flag_set_bits(flag_handle_mutex_demo_queue_non_empty, DEMO_QUEUE_NON_EMPTY, SET_FLAG_MODE_OR);
    }

    // 发送线程已完成标记
    ks_os_flag_set_bits(flag_handle_mutex_demo_finish,
                        (thread_id == 1 ? DEMO_FINISHED_1 : DEMO_FINISHED_2),
                        SET_FLAG_MODE_OR);
    ks_os_flag_set_bits(flag_handle_mutex_demo_queue_non_empty,
                        (thread_id == 1 ? DEMO_QUEUE_EXIT_1 : DEMO_QUEUE_EXIT_2),
                        SET_FLAG_MODE_OR);
}

void MutexDemo(void)
{
    U32 ret;
    ks_driver_uart_send_string(0, "-------------------------------\r\n");
    ks_driver_uart_send_string(0, "----Mutex Demo begins----------\r\n");

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_mutex_demo_finish, "flag_mutex_demo_finish");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建互斥量
    ret = ks_os_mutex_create(&mutex_handle_mutex_demo, "mutex_mutex_demo");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating mutex!\r\n");
        return;
    }
    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_mutex_demo_queue_non_empty, "flag_mutex_demo_queue_non_empty");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_mutex_demo_task_1,
                              "mutex_demo_generator1",
                              MutexDemoGenerator,
                              (void *) 1,
                              18,
                              thread_stack_mutex_demo_task_1,
                              sizeof(thread_stack_mutex_demo_task_1),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }
    ret = ks_os_thread_create(&thread_handle_mutex_demo_task_2,
                              "mutex_demo_generator2",
                              MutexDemoGenerator,
                              (void *) 2,
                              19,
                              thread_stack_mutex_demo_task_2,
                              sizeof(thread_stack_mutex_demo_task_2),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }
    ret = ks_os_thread_create(&thread_handle_mutex_demo_task_3,
                              "mutex_demo_consumer1",
                              MutexDemoConsumer,
                              (void *) 1,
                              20,
                              thread_stack_mutex_demo_task_3,
                              sizeof(thread_stack_mutex_demo_task_3),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }
    ret = ks_os_thread_create(&thread_handle_mutex_demo_task_4,
                              "mutex_demo_consumer2",
                              MutexDemoConsumer,
                              (void *) 2,
                              21,
                              thread_stack_mutex_demo_task_4,
                              sizeof(thread_stack_mutex_demo_task_4),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    // 等待标识量（Demo线程完成）
    ks_os_flag_wait(flag_handle_mutex_demo_finish,
                    DEMO_FINISHED_1 | DEMO_FINISHED_2 | DEMO_FINISHED_3 | DEMO_FINISHED_4,
                    WAIT_FLAG_MODE_AND_CLEAR, KS_OS_WAIT_FOREVER);

    ks_driver_uart_send_string(0, "----Mutex Demo ends------------\r\n");
}

