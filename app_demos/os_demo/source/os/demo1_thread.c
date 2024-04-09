
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  demo1_thread.c
 *
 * @brief  os 测试例程: 线程
 */


#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "os_demo.h"

#define  STACK_SIZE_THREAD_DEMO          512
static S32 thread_stack_thread_demo_task[STACK_SIZE_THREAD_DEMO];
static OSHandle thread_handle_thread_demo_task;

#define DEMO_FINISHED 1
static OSHandle flag_handle_thread_demo_finish;

static S32 thread_stack_thread_demo_test_task_1[STACK_SIZE_THREAD_DEMO];
static OSHandle thread_handle_thread_demo_test_task_1;

static S32 thread_stack_thread_demo_test_task_2[STACK_SIZE_THREAD_DEMO];
static OSHandle thread_handle_thread_demo_test_task_2;



void ThreadDemoTestTask(void *p_arg)
{
    char print_buffer[128];
    ks_os_thread_vfp_enable();
    
    S32 task_id = (S32)p_arg;
    for (int i = 1; i <= 10; ++i)
    {
        // 线程休眠50ms
        ks_os_thread_sleep(5);
        // 输出信息
        sprintf(print_buffer, "Test thread %d: print %d\r\n", task_id, i);
        ks_driver_uart_send_string(0, print_buffer);
    }
}


void ThreadDemoTask(void *p_arg)
{
    // 使能浮点运算
    ks_os_thread_vfp_enable();

    S32 task_id;
    U32 ret;
    char print_buffer[64];

    // 创建线程1，自动启动
    task_id = 1;
    ret = ks_os_thread_create(&thread_handle_thread_demo_test_task_1,
                              "thread_demo_test_task_1",
                              ThreadDemoTestTask,
                              (void*)task_id,
                              19,
                              thread_stack_thread_demo_test_task_1,
                              sizeof(thread_stack_thread_demo_test_task_1),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    // 创建线程2，不自动启动
    task_id = 2;
    ret = ks_os_thread_create(&thread_handle_thread_demo_test_task_2,
                              "thread_demo_test_task_2",
                              ThreadDemoTestTask,
                              (void*)task_id,
                              20,
                              thread_stack_thread_demo_test_task_2,
                              sizeof(thread_stack_thread_demo_test_task_2),
                              0,
                              0
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    // 等待210ms
    ks_os_thread_sleep(21);
    // 恢复线程2
    ks_driver_uart_send_string(0, "--Resume thread 2\r\n");
    ks_os_thread_resume(thread_handle_thread_demo_test_task_2);
    // 等待200ms
    ks_os_thread_sleep(20);
    // 暂停线程1
    ks_driver_uart_send_string(0, "--Suspend thread 1\r\n");
    ks_os_thread_suspend(thread_handle_thread_demo_test_task_1);
    // 等待200ms
    ks_os_thread_sleep(20);
    // 恢复线程1
    ks_driver_uart_send_string(0, "--Resume thread 1\r\n");
    ks_os_thread_resume(thread_handle_thread_demo_test_task_1);
    // 等待200ms
    ks_os_thread_sleep(20);

    // 发送线程已完成标记
    ks_os_flag_set_bits(flag_handle_thread_demo_finish, DEMO_FINISHED, SET_FLAG_MODE_OR);
}


void ThreadDemo(void)
{
    U32 ret;
    char print_buffer[64];
    ks_driver_uart_send_string(0, "-------------------------------\r\n");
    ks_driver_uart_send_string(0, "----Thread Demo begins---------\r\n");

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_thread_demo_finish, "flag_thread_demo_finish");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_thread_demo_task,        // 线程句柄
                              "thread_demo_task",                     // 线程名，不能重复
                              ThreadDemoTask,                         // 线程入口函数指针
                              0,                                      // 线程入口函数参数
                              21,                                     // 线程优先级，数字越小越高，推荐用户使用8~23
                              thread_stack_thread_demo_task,          // 线程栈指针
                              sizeof(thread_stack_thread_demo_task),  // 线程栈大小
                              0,                                      // 时间片
                              1                                       // 是否自动启动
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    // 等待标识量（Demo线程完成）
    ks_os_flag_wait(flag_handle_thread_demo_finish, DEMO_FINISHED, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);

    ks_driver_uart_send_string(0, "----Thread Demo ends-----------\r\n");
}

