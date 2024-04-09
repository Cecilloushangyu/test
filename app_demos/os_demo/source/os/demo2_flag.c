
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  demo2_flag.c
 *
 * @brief  os 测试例程: 标识量
 */


#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "os_demo.h"

#define  STACK_SIZE_FLAG_DEMO          512
static S32 thread_stack_flag_demo_task_1[STACK_SIZE_FLAG_DEMO];
static OSHandle thread_handle_flag_demo_task_1;

static S32 thread_stack_flag_demo_task_2[STACK_SIZE_FLAG_DEMO];
static OSHandle thread_handle_flag_demo_task_2;

#define DEMO_FINISHED_1 1
#define DEMO_FINISHED_2 2
static OSHandle flag_handle_flag_demo_finish;

#define FLAG_1 1
#define FLAG_2 (1<<1)
#define FLAG_3 (1<<2)
#define FLAG_END (1<<31)
static OSHandle flag_handle_demo;


void FlagDemoTask2(void *p_arg)
{
    char print_buffer[128];
    U32 flag;
    ks_os_thread_vfp_enable();

    while(1)
    {
        // 等待标识量
        flag = ks_os_flag_wait(flag_handle_demo, FLAG_1|FLAG_2|FLAG_3|FLAG_END, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);

        if (flag & FLAG_END)
        {
            // 线程结束
            sprintf(print_buffer, "Thread ends\r\n", flag);
            ks_driver_uart_send_string(0, print_buffer);
            // 发送线程已完成标记
            ks_os_flag_set_bits(flag_handle_flag_demo_finish, DEMO_FINISHED_2, SET_FLAG_MODE_OR);
            return;
        }

        // 输出信息
        sprintf(print_buffer, "Receive FLAG %d\r\n", flag);
        ks_driver_uart_send_string(0, print_buffer);
    }
}


void FlagDemoTask1(void *p_arg)
{
    U32 ret;
    // 使能浮点运算
    ks_os_thread_vfp_enable();

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_demo, "flag_demo");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_flag_demo_task_2,
                              "flag_demo_task2",
                              FlagDemoTask2,
                              0,
                              20,
                              thread_stack_flag_demo_task_2,
                              sizeof(thread_stack_flag_demo_task_2),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    // 标记FLAG1
    ks_os_flag_set_bits(flag_handle_demo, FLAG_1, SET_FLAG_MODE_OR);
    // 等待10ms
    ks_os_thread_sleep(1);
    // 标记FLAG2
    ks_os_flag_set_bits(flag_handle_demo, FLAG_2, SET_FLAG_MODE_OR);
    // 等待10ms
    ks_os_thread_sleep(1);
    // 标记FLAG3
    ks_os_flag_set_bits(flag_handle_demo, FLAG_3, SET_FLAG_MODE_OR);
    // 等待10ms
    ks_os_thread_sleep(1);
    // 同时标记FLAG1、FLAG2、FLAG3
    ks_os_flag_set_bits(flag_handle_demo, FLAG_1|FLAG_2|FLAG_3, SET_FLAG_MODE_OR);
    // 等待10ms
    ks_os_thread_sleep(1);
    // 发送结束信号
    ks_os_flag_set_bits(flag_handle_demo, FLAG_END, SET_FLAG_MODE_OR);

    // 发送线程已完成标记
    ks_os_flag_set_bits(flag_handle_flag_demo_finish, DEMO_FINISHED_1, SET_FLAG_MODE_OR);
}


void FlagDemo(void)
{
    U32 ret;
    ks_driver_uart_send_string(0, "-------------------------------\r\n");
    ks_driver_uart_send_string(0, "----Flag Demo begins-----------\r\n");

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_flag_demo_finish, "flag_flag_demo_finish");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_flag_demo_task_1,
                              "flag_demo_task1",
                              FlagDemoTask1,
                              0,
                              21,
                              thread_stack_flag_demo_task_1,
                              sizeof(thread_stack_flag_demo_task_1),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    // 等待标识量（Demo线程完成）
    ks_os_flag_wait(flag_handle_flag_demo_finish, DEMO_FINISHED_1|DEMO_FINISHED_2, WAIT_FLAG_MODE_AND_CLEAR, KS_OS_WAIT_FOREVER);

    ks_driver_uart_send_string(0, "----Flag Demo ends-------------\r\n");
}

