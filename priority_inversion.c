/* 
 * Copyright (c) 2006-2018, RT-Thread Development Team 
 * 
 * SPDX-License-Identifier: Apache-2.0 
 * 
 * Change Logs: 
 * Date           Author       Notes 
 * 2018-08-24     yangjie      the first version 
 */ 

/*
 * 程序清单：互斥量使用例程
 *
 * 这个例子将创建 3 个动态线程以检查持有互斥量时，持有的线程优先级是否
 * 被调整到等待线程优先级中的最高优先级。
 *
 * 线程 1，2，3 的优先级从高到低分别被创建，
 * 线程 3 先持有互斥量然后运行500ms，
 * 而后线程 2 试图持有互斥量，此时线程 3 的优先级应该被提升为和线程 2 的优先级相同。
 * 线程 1 用于检查线程 3 的优先级是否被提升为与线程 2的优先级相同。
 * 修改了官方例程，增加若干打印信息。
 * 增加一个输入参数，控制线程3是否先持有互斥量（参数为0或非数字字符，则让线程3不先持有互斥量，于是便不会产生优先级继承现象）
 */
#include <rtthread.h>

/* 指向线程控制块的指针 */
static rt_thread_t tid1 = RT_NULL;
static rt_thread_t tid2 = RT_NULL;
static rt_thread_t tid3 = RT_NULL;
static rt_mutex_t mutex = RT_NULL;


#define THREAD_PRIORITY       10
#define THREAD_STACK_SIZE     512
#define THREAD_TIMESLICE      5

/* 线程 1 入口 */
static void thread1_entry(void *parameter)
{
    /* 先让低优先级线程运行 */
    rt_kprintf("T1 entry: blocked 100ms.\n");
    rt_thread_mdelay(100);

    /* 此时 thread3 持有 mutex，并且 thread2 等待持有 mutex */
    rt_kprintf("T1 testing:\n");
    /* 检查 rt_kprintf("the producer generates a number: %d\n", array[set%MAXSEM]); 与 thread3 的优先级情况 */
    if (tid2->current_priority != tid3->current_priority)
    {
        /* 优先级不相同，测试失败 */
        rt_kprintf(" - Priority of T2 is: %d\n", tid2->current_priority);
        rt_kprintf(" - Priority of T3 is: %d\n", tid3->current_priority);
        rt_kprintf(" - Test failed.\n");
        return;
    }
    else
    {
        rt_kprintf(" - Priority of T2 is: %d\n", tid2->current_priority);
        rt_kprintf(" - Priority of T3 is: %d\n", tid3->current_priority);
        rt_kprintf(" - Test OK.\n");
    }
}

/* 线程 2 入口 */
static void thread2_entry(void *parameter)
{
    rt_tick_t tick;
    rt_err_t result;

    //rt_kprintf("the priority of thread2 is: %d\n", tid2->current_priority);
    rt_kprintf("T2 entry: blocked 50ms.\n");
    /* 先让低优先级线程运行 */
    rt_thread_mdelay(50);

    /*
     * 试图持有互斥锁，此时 thread3 持有，应把 thread3 的优先级提升
     * 到 thread2 相同的优先级
     */
    rt_kprintf("T2 try to take a mutex.\n");
    result = rt_mutex_take(mutex, RT_WAITING_FOREVER);
    if (result != RT_EOK)
    {
        rt_kprintf("T2 take a mutex, failed.\n");
    }
    else
    {
        rt_kprintf("T2 take a mutex successed.\n");
    }
    /* 做一个长时间的循环，500ms */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < (RT_TICK_PER_SECOND / 2)) {};

    if (result == RT_EOK)
    {
        rt_kprintf("T2 release mutex and exit.\n");
        rt_mutex_release(mutex);
    }
    
}

/* 线程 3 入口 */
static void thread3_entry(void *parameter)
{
    rt_uint32_t flag;
    rt_tick_t tick;
    rt_err_t result;

    flag = (rt_uint32_t) parameter;
    //rt_kprintf("the priority of thread3 is: %d\n", tid3->current_priority);
    rt_kprintf("T3 entry: no blocked.\n");
    if (flag)
    {
        rt_kprintf("T3 try to take a mutex.\n");
        result = rt_mutex_take(mutex, RT_WAITING_FOREVER);
        if (result != RT_EOK)
        {
            rt_kprintf("T3 take a mutex, failed.\n");
        }
        else
        {
            rt_kprintf("T3 take a mutex successed.\n");
        }
    }
    /* 做一个长时间的循环，500ms */
    tick = rt_tick_get();
    while (rt_tick_get() - tick < (RT_TICK_PER_SECOND / 2)) {};

    if (flag && result == RT_EOK)
    {
        rt_kprintf("T3 release mutex and exit.\n");
        rt_mutex_release(mutex);
    }
}

int pri_inversion(int argc, char**argv)
{
    extern rt_uint32_t str2dec(char* str);
    rt_uint32_t flag;
    
    if(argc<=1)
    {
        flag = 1;
    }
    else
    {
        flag = (str2dec(argv[1])!=0);
    }
    
    
    /* 创建互斥锁 */
    mutex = rt_mutex_create("mutex", RT_IPC_FLAG_FIFO);
    if (mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }

    /* 创建线程 1 ，优先级最高，会延迟一段时间后，检查线程2与3的优先级。*/
    
    tid1 = rt_thread_create("thread1",
                            thread1_entry, 
                            RT_NULL,
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY - 1, THREAD_TIMESLICE);
    if (tid1 != RT_NULL)
    {
        rt_kprintf("T1 created, with priority %d\n", tid1->current_priority);
        rt_thread_startup(tid1);
    }
 
    /* 创建线程 2 ，优先级中等，一定会尝试获取互斥量。*/
    tid2 = rt_thread_create("thread2",
                            thread2_entry, 
                            RT_NULL, 
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, THREAD_TIMESLICE);
    if (tid2 != RT_NULL)
    {
        rt_kprintf("T2 created, with priority %d\n", tid2->current_priority);
        rt_thread_startup(tid2);
    }

    /* 创建线程 3 ，优先级最低；会根据输入变量决定是否尝试获取互斥量。*/
    tid3 = rt_thread_create("thread3",
                            thread3_entry, 
                            (void*)flag,
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY + 1, THREAD_TIMESLICE);
    if (tid3 != RT_NULL)
    {
        rt_kprintf("T3 created, with priority %d\n", tid3->current_priority);
        rt_thread_startup(tid3);
    }

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(pri_inversion, pri_inversion sample);
