
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  demo7_irq.c
 *
 * @brief  os 测试例程: IRQ屏蔽与恢复
 */


#include <string.h>
#include "ks_include.h"
#include "ks_uart.h"
#include "os_demo.h"

#define DEMO_IRQ_FINISH 1

#define  STACK_SIZE_IRQ_DEMO          512
static S32 thread_stack_irq_demo_task[STACK_SIZE_IRQ_DEMO];
static OSHandle thread_handle_irq_demo_task;
static OSHandle flag_handle_irq_demo;
static int print_count = 0;

void IRQDemoTask(void *p_arg)
{
    char buffer[128];
    ks_os_thread_vfp_enable();
    while (1)
    {
        U32 flag = ks_os_flag_wait(flag_handle_irq_demo, DEMO_IRQ_FINISH, WAIT_FLAG_MODE_OR_CLEAR, 100);
        if (flag & DEMO_IRQ_FINISH)
            break;
        print_count++;
        sprintf(buffer, "Print in high priority task: #%d\r\n", print_count);
        ks_driver_uart_poll_send_string(0, buffer);
    }
}

S32 UARTCallback(S32 uart_id, char c)
{
    ks_driver_uart_poll_send_byte(0, c);
    return 0;
}

void IRQDemo(void)
{
    U32 ret;
    U64 time_cycles_cur, time_cycles_total;
    DOUBLE time_sec;
    U32 flag;
    char print_buffer[128];
    ks_driver_uart_send_string(0, "-------------------------------\r\n");
    ks_driver_uart_send_string(0, "----IRQ Demo begins------------\r\n");
    ks_os_thread_sleep(1);

    // 创建标识量
    ret = ks_os_flag_create(&flag_handle_irq_demo, "flag_irq_demo");
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating flag!\r\n");
        return;
    }
    // 创建线程
    ret = ks_os_thread_create(&thread_handle_irq_demo_task,
                              "irq_demo_task",
                              IRQDemoTask,
                              0,
                              8,
                              thread_stack_irq_demo_task,
                              sizeof(thread_stack_irq_demo_task),
                              0,
                              1
    );
    if (ret != OSHANDLE_SUCCESS)
    {
        ks_driver_uart_send_string(0, "Error creating thread!\r\n");
        return;
    }

    ks_driver_uart_register_protocol(0, UARTCallback);

    ks_driver_uart_poll_send_string(0, "Try sending content to UART0\r\n");
    for (int i = 0; i < 10; ++i)
    {
        ks_driver_uart_poll_send_string(0, "-- Disable IRQ\r\n");
        // 屏蔽中断
        ks_os_irq_mask_all();
        // 延时5s
        ks_os_poll_delay_msec(5000);
        ks_driver_uart_poll_send_string(0, "-- Enable IRQ\r\n");
        // 取消屏蔽中断
        ks_os_irq_unmask_all();
        // 延时5s
        ks_os_poll_delay_msec(5000);
    }

    ks_os_flag_set_bits(flag_handle_irq_demo, DEMO_IRQ_FINISH, SET_FLAG_MODE_OR);

    ks_driver_uart_poll_send_string(0, "----IRQ Demo ends--------------\r\n");
}

