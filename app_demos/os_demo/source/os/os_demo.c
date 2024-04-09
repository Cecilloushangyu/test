
/**
 * Copyright (tm) 2022, KiSilicon,
 * All rights reversed.
 *
 * @file  os_demo.c
 *
 * @brief  os 测试例程
 */


#include "os_demo.h"
#include "ks_datatypes.h"


#define STACK_SIZE_OS_DEMO        512
static S32 thread_stack_run_demo[STACK_SIZE_OS_DEMO];
static OSHandle thread_handle_run_demo;

void RunOSDemoTask(void *p_arg)
{
    // 允许浮点计算
    ks_os_thread_vfp_enable();

    // 运行各个子例程，各子例程相互独立，可通过注释掉部分语句手动选择是否运行
    ThreadDemo();
    FlagDemo();
    SemaphoreDemo();
    MailboxDemo();
    MutexDemo();
    TimingDemo();
    IRQDemo();
}

void RunOsDemo(void)
{
    ks_os_thread_create(&thread_handle_run_demo,
                        "run_demo_task",
                        RunOSDemoTask,
                        0,
                        22,
                        thread_stack_run_demo,
                        sizeof(thread_stack_run_demo),
                        0,
                        1
    );
}

