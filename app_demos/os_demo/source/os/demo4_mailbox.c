
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  demo4_mailbox.c
 *
 * @brief  os 测试例程: 邮箱
 */


#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "os_demo.h"

#define DEMO_FINISHED_1 1
#define DEMO_FINISHED_2 2

static OSHandle flag_handle_mbx_demo_finish;

#define  STACK_SIZE_MBX_DEMO          512
static S32 thread_stack_mbx_demo_task_1[STACK_SIZE_MBX_DEMO];
static OSHandle thread_handle_mbx_demo_task_1;

static S32 thread_stack_mbx_demo_task_2[STACK_SIZE_MBX_DEMO];
static OSHandle thread_handle_mbx_demo_task_2;

#define MBX_QUEUE_SIZE 16
static U32 mbx_queue[MBX_QUEUE_SIZE];
static OSHandle mbx_handle_mbx_demo;


void MailboxDemoTask2(void *p_arg)
{
    char print_buffer[128];
    S32 sleep_interval = 7;
    U32 current_sem_count;
    char* mail_content;

    ks_os_thread_vfp_enable();

    for (S32 i = 0; i < 5; ++i) {
        // 等待生产完成
        ks_os_mbx_pend(mbx_handle_mbx_demo, (void **) &mail_content, KS_OS_WAIT_FOREVER);
        sprintf(print_buffer, "Consume #%d -- %s\r\n", i+1, mail_content);
        ks_driver_uart_send_string(0, print_buffer);
        // 睡眠间隔时间，模拟消费者消费时间
        ks_os_thread_sleep(sleep_interval);
    }

    // 发送线程已完成标记
    ks_os_flag_set_bits(flag_handle_mbx_demo_finish, DEMO_FINISHED_2, SET_FLAG_MODE_OR);
}


void MailboxDemoTask1(void *p_arg)
{
    char print_buffer[128];
    // 生产者生产时间间隔
    S32 sleep_intervals[] = {
        1, 2, 3, 4, 5
    };
    const char* contents[5] = {
        "Content1: abc",
        "Content2: xxx",
        "Content3: asdf",
        "Content4: 1234",
        "Content5: 00000",
    };
    char* mail_content;

    // 使能浮点运算
    ks_os_thread_vfp_enable();

    for (S32 i = 0; i < 5; ++i) {
        // 睡眠间隔时间，模拟生产者生产时间
        ks_os_thread_sleep(sleep_intervals[i]);
        // 通知生产完成
        mail_content = (char*)contents[i];
        ks_os_mbx_post(mbx_handle_mbx_demo, (void *)&mail_content);
        sprintf(print_buffer, "Generate #%d\r\n", i+1);
        ks_driver_uart_send_string(0, print_buffer);
    }

    // 发送线程已完成标记
    ks_os_flag_set_bits(flag_handle_mbx_demo_finish, DEMO_FINISHED_1, SET_FLAG_MODE_OR);
}


void MailboxDemo(void)
{
    U32 ret;
    ks_driver_uart_send_string(0, "-------------------------------\r\n");
    ks_driver_uart_send_string(0, "----Mailbox Demo begins--------\r\n");

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_mbx_demo_finish, "flag_mbx_demo_finish");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建邮箱
    ret = ks_os_mbx_create(&mbx_handle_mbx_demo, "mbx_mbx_demo", mbx_queue, MBX_QUEUE_SIZE);
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating mailbox!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_mbx_demo_task_1,
                              "mbx_demo_task1",
                              MailboxDemoTask1,
                              0,
                              20,
                              thread_stack_mbx_demo_task_1,
                              sizeof(thread_stack_mbx_demo_task_1),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }
    ret = ks_os_thread_create(&thread_handle_mbx_demo_task_2,
                              "mbx_demo_task2",
                              MailboxDemoTask2,
                              0,
                              21,
                              thread_stack_mbx_demo_task_2,
                              sizeof(thread_stack_mbx_demo_task_2),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    // 等待标识量（Demo线程完成）
    ks_os_flag_wait(flag_handle_mbx_demo_finish, DEMO_FINISHED_1|DEMO_FINISHED_2, WAIT_FLAG_MODE_AND_CLEAR, KS_OS_WAIT_FOREVER);

    ks_driver_uart_send_string(0, "----Mailbox Demo ends----------\r\n");
}

