
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  demo6_timing.c
 *
 * @brief  os 测试例程: 时间相关API
 */


#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "os_demo.h"

#define DEMO_TIMEOUT 1

static OSHandle flag_handle_timing_demo_timeout;
static OSHandle timer_handle_timing_demo_coarse;

#define DEMO_TIME_START 1
#define DEMO_TIME_END 2
#define DEMO_TIME_EXIT (1<<31)
static OSHandle flag_handle_timing_demo_timing_stat;
#define DEMO_FINISH 1
static OSHandle flag_handle_timing_demo_finish;

#define  STACK_SIZE_TIMING_DEMO          512
static S32 thread_stack_timing_demo_timing_task[STACK_SIZE_TIMING_DEMO];
static OSHandle thread_handle_timing_demo_timing_task;
static S32 thread_stack_timing_demo_work_task[STACK_SIZE_TIMING_DEMO];
static OSHandle thread_handle_timing_demo_work_task;

void TimerCallback(void* arg)
{
    // 发送超时标记
    ks_os_flag_set_bits(flag_handle_timing_demo_timeout, DEMO_TIMEOUT, SET_FLAG_MODE_OR);
}

void TimingDemoWorkTask(void *p_arg)
{
    char print_buffer[128];
    ks_os_thread_vfp_enable();

    // 通知开始计时
    ks_os_flag_set_bits(flag_handle_timing_demo_timing_stat, DEMO_TIME_START, SET_FLAG_MODE_OR);

    ks_driver_uart_send_string(0, "Delay for 1s\r\n");

    // 用delay函数等待1s
    ks_os_poll_delay_msec(1000);

    // 通知停止计时
    ks_os_flag_set_bits(flag_handle_timing_demo_timing_stat, DEMO_TIME_END, SET_FLAG_MODE_OR);

    // 通知开始计时
    ks_os_flag_set_bits(flag_handle_timing_demo_timing_stat, DEMO_TIME_START, SET_FLAG_MODE_OR);

    ks_driver_uart_send_string(0, "Sleep for 1s\r\n");

    // 用sleep函数等待1s
    ks_os_thread_sleep(100);

    // 通知停止计时
    ks_os_flag_set_bits(flag_handle_timing_demo_timing_stat, DEMO_TIME_END, SET_FLAG_MODE_OR);

    // 通知退出
    ks_os_flag_set_bits(flag_handle_timing_demo_timing_stat, DEMO_TIME_EXIT, SET_FLAG_MODE_OR);

    // 通知线程结束
    ks_os_flag_set_bits(flag_handle_timing_demo_finish, DEMO_FINISH, SET_FLAG_MODE_OR);
}

void TimingDemoTimingTask(void *p_arg)
{
    ks_os_thread_vfp_enable();

    // 获取时钟
    DOUBLE sys_clock = ks_os_get_sys_clock();

    char print_buffer[128];
    U32 flag;
    U64 thread_time, irq_time, idle_time;
    U64 thread_time_diff, irq_time_diff, idle_time_diff;
    thread_time = irq_time = idle_time = 0;
    thread_time_diff = irq_time_diff = idle_time_diff = 0;
    while (1)
    {
        // 等待标识量到来
        flag = ks_os_flag_wait(flag_handle_timing_demo_timing_stat,
                               DEMO_TIME_START | DEMO_TIME_END | DEMO_TIME_EXIT, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);

        // 待计时线程
        OSHandle thread_handle = thread_handle_timing_demo_work_task;

        // 停止计时
        if (flag & DEMO_TIME_END)
        {
            thread_time_diff = ks_os_load_get_thread_time(thread_handle) - thread_time;
            irq_time_diff = ks_os_load_get_irq_time() - irq_time;
            idle_time_diff = ks_os_load_get_idle_time() - idle_time;

            sprintf(print_buffer,
                    "Thread time diff: %.6fs; IRQ time diff: %.6fs; idle time diff: %.6fs\r\n",
                    (DOUBLE) thread_time_diff / sys_clock,
                    (DOUBLE) irq_time_diff / sys_clock,
                    (DOUBLE) idle_time_diff / sys_clock);
            ks_driver_uart_send_string(0, print_buffer);
        }
        // 开始计时
        if (flag & DEMO_TIME_START)
        {
            thread_time = ks_os_load_get_thread_time(thread_handle);
            irq_time = ks_os_load_get_irq_time();
            idle_time = ks_os_load_get_idle_time();
        }
        // 退出
        if (flag & DEMO_TIME_EXIT)
        {
            break;
        }
    }
}


void TimingDemo(void)
{
    U32 ret;
    U64 time_cur, time_total;
    DOUBLE time_sec;
    U32 flag;
    char print_buffer[128];
    ks_driver_uart_send_string(0, "-------------------------------\r\n");
    ks_driver_uart_send_string(0, "----Timing Demo begins---------\r\n");

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_timing_demo_timeout, "flag_timing_demo_timeout");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }

    // 获取时钟
    DOUBLE sys_clock = ks_os_get_sys_clock();
    DOUBLE apb_clock = ks_os_get_apb_clock();
    DOUBLE ahb_clock = ks_os_get_ahb_clock();
    sprintf(print_buffer, "SYS CLK: %.6fMHz; APB CLK: %.6fMHz; AHB CLK: %.6fMhz\r\n",
            sys_clock / 1e6, apb_clock / 1e6, ahb_clock / 1e6);
    ks_driver_uart_send_string(0, print_buffer);

    // 设置和查询时间
    ks_os_chrono_set_time(0);
    time_cur = ks_os_chrono_current_time();
    time_total = ks_os_chrono_elapsed_time();
    sprintf(print_buffer, "After set time to 0, current time: %.6fs;\r\n"
                          "total time: %.6fs\r\n",
                          (DOUBLE) time_cur / 1e9,
                          (DOUBLE) time_total / 1e9);
    ks_driver_uart_send_string(0, print_buffer);

    // 延时
    ks_os_chrono_set_time(0);
    ks_os_poll_delay_msec(1);
    time_cur = ks_os_chrono_current_time();
    sprintf(print_buffer, "Delay 1 msec, time diff: %.6fs\r\n", (DOUBLE) time_cur / 1e9);
    ks_driver_uart_send_string(0, print_buffer);

    ks_os_chrono_set_time(0);
    ks_os_poll_delay_usec(100);
    time_cur = ks_os_chrono_current_time();
    sprintf(print_buffer, "Delay 100 usec, time diff: %.6fs\r\n", (DOUBLE) time_cur / 1e9);
    ks_driver_uart_send_string(0, print_buffer);

    // 粗略计时器
    ret = ks_os_timer_coarse_create(&timer_handle_timing_demo_coarse,   // 句柄指针
                                    "timer_timing_demo_coarse",         // 名称
                                    TimerCallback,                      // 回调函数
                                    0,                                  // 回调函数参数
                                    20,                                 // 初始设定
                                    20,                                 // 定时间隔
                                    1                                   // 是否自动启动
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating timer!\r\n");
        return;
    }
    ks_os_chrono_set_time(0);
    ks_driver_uart_send_string(0, "-- Cur time set to 0\r\n");
    ks_driver_uart_send_string(0, "-- Start coarse timer, interval 20ms\r\n");
    for (int i = 0; i < 3; ++i)
    {
        // 等待标识量（超时）
        ks_os_flag_wait(flag_handle_timing_demo_timeout, DEMO_TIMEOUT, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
        time_cur = ks_os_chrono_current_time();
        sprintf(print_buffer, "Timer timeout, cur time: %.6fs\r\n", (DOUBLE) time_cur / 1e9);
        ks_driver_uart_send_string(0, print_buffer);
    }
    // 停止计时器
    ks_os_timer_coarse_deactivate(timer_handle_timing_demo_coarse);
    // 修改计时器
    ks_os_timer_coarse_change(timer_handle_timing_demo_coarse, TimerCallback, 0, 10, 30);
    ks_driver_uart_send_string(0, "-- Change coarse timer, first time 10ms, interval 30ms\r\n");
    // 启动计时器
    ks_os_timer_coarse_activate(timer_handle_timing_demo_coarse);
    {
        U32 remaining_time;
        ks_os_timer_coarse_info_get(timer_handle_timing_demo_coarse, NULL, NULL, &remaining_time, NULL);
        sprintf(print_buffer, "Time to till next timeout: %dms\r\n", remaining_time);
        ks_driver_uart_send_string(0, print_buffer);
    }
    for (int i = 0; i < 3; ++i)
    {
        // 等待标识量（超时）
        ks_os_flag_wait(flag_handle_timing_demo_timeout, DEMO_TIMEOUT, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
        time_cur = ks_os_chrono_current_time();
        sprintf(print_buffer, "Timer timeout, cur time: %.6fs\r\n", (DOUBLE) time_cur / 1e9);
        ks_driver_uart_send_string(0, print_buffer);
        U32 remaining_time;
        ks_os_timer_coarse_info_get(timer_handle_timing_demo_coarse, NULL, NULL, &remaining_time, NULL);
        sprintf(print_buffer, "Time to till next timeout: %dms\r\n", remaining_time);
        ks_driver_uart_send_string(0, print_buffer);
    }
    // 停止计时器
    ks_os_timer_coarse_deactivate(timer_handle_timing_demo_coarse);

    // 精确计时器
    ret = ks_os_timer_accurate_create(TimerCallback,                      // 回调函数
                                      0,                                  // 回调函数参数
                                      200,                                // 初始设定
                                      200,                                // 定时间隔
                                      1                                   // 是否自动启动
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating timer!\r\n");
        return;
    }
    ks_os_chrono_set_time(0);
    ks_driver_uart_send_string(0, "-- Cur time set to 0\r\n");
    ks_driver_uart_send_string(0, "-- Start accurate timer, interval 200us\r\n");
    for (int i = 0; i < 3; ++i)
    {
        // 等待标识量（超时）
        ks_os_flag_wait(flag_handle_timing_demo_timeout, DEMO_TIMEOUT, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
        time_cur = ks_os_chrono_current_time();
        sprintf(print_buffer, "Timer timeout, cur time: %.6fs\r\n", (DOUBLE) time_cur / 1e9);
        ks_driver_uart_send_string(0, print_buffer);
    }
    // 停止计时器
    ks_os_timer_accurate_deactivate();
    // 修改计时器
    ks_os_timer_accurate_change(TimerCallback, 0, 100, 300);
    ks_driver_uart_send_string(0, "-- Change accurate timer, first time 100us, interval 300us\r\n");
    // 启动计时器
    ks_os_timer_accurate_activate();
    {
        DOUBLE remaining_time;
        ks_os_timer_accurate_info_get(NULL, &remaining_time, NULL);
        sprintf(print_buffer, "Time to till next timeout: %fus\r\n", remaining_time);
        ks_driver_uart_send_string(0, print_buffer);
    }
    for (int i = 0; i < 3; ++i)
    {
        // 等待标识量（超时）
        ks_os_flag_wait(flag_handle_timing_demo_timeout, DEMO_TIMEOUT, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);
        time_cur = ks_os_chrono_current_time();
        sprintf(print_buffer, "Timer timeout, cur time: %.6fs\r\n", (DOUBLE) time_cur / 1e9);
        ks_driver_uart_send_string(0, print_buffer);
        DOUBLE remaining_time;
        ks_os_timer_accurate_info_get(NULL, &remaining_time, NULL);
        sprintf(print_buffer, "Time to till next timeout: %fus\r\n", remaining_time);
        ks_driver_uart_send_string(0, print_buffer);
    }
    // 停止计时器
    ks_os_timer_accurate_deactivate();

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_timing_demo_timing_stat, "flag_timing_stat");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_timing_demo_finish, "flag_timing_finish");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_timing_demo_timing_task,
                              "timing_demo_timing_task",
                              TimingDemoTimingTask,
                              0,
                              10,       // 注意计时线程可适当选择高优先级
                              thread_stack_timing_demo_timing_task,
                              sizeof(thread_stack_timing_demo_timing_task),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_timing_demo_work_task,
                              "timing_demo_work_task",
                              TimingDemoWorkTask,
                              0,
                              20,
                              thread_stack_timing_demo_work_task,
                              sizeof(thread_stack_timing_demo_work_task),
                              0,
                              0
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }
    // 由于在其他高优先级线程中用到了线程句柄，使用先创建线程再启动的方式确认句柄已被赋值
    ks_os_thread_resume(thread_handle_timing_demo_work_task);

    // 等待线程结束
    flag = ks_os_flag_wait(flag_handle_timing_demo_finish, DEMO_FINISH, WAIT_FLAG_MODE_OR_CLEAR, KS_OS_WAIT_FOREVER);

    ks_driver_uart_send_string(0, "----Timing Demo ends-----------\r\n");
}

