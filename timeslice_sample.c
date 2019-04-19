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
 * 程序清单：相同优先级线程按照时间片轮番调度
 *
 * 这个例子中将创建两个线程，但是其实它们是同一个函数，功能就是定期打印信息。
 * 线程1打印了一段时间后，时间片到，切换到线程2，如此往复；观察每个线程打印到多少，就可以观察到时间片效果。
 * 改写了官方例程部分参数。现在函数运行时间更长，同时打印时加入了暂停调度临界区保护，避免打到一半被切换影响观察效果。
 * 同时，增加输入参数，以自定义两个线程的时间片。
 */

#include <rtthread.h>

#define THREAD_STACK_SIZE	1024
#define THREAD_PRIORITY	    20
#define THREAD_TIMESLICE    30

/* 线程入口 */ 
static void thread_entry(void* parameter)
{
    rt_uint32_t value;
    rt_uint32_t count = 0;
    value = (rt_uint32_t)parameter; /* 通过输入参数作区别 */
    while (1)
    {
        if(0 == (count % 10))
        {
            rt_enter_critical();
            rt_kprintf("thread %d counting:%d\n", value , count);
            rt_exit_critical();	
            if(count >= 6000)
            {
                break;
            }
        }
         count++;
         for(volatile int i=0;i<2019;++i){;} /* 进一步增加延迟 */
     }
    rt_enter_critical();
    rt_kprintf("\nthread%d end\n\n", value);
    rt_exit_critical();	
    return;
}

int timeslice_sample(int argc, char**argv)
{
    extern rt_uint32_t str2dec(char* str);
    rt_thread_t tid; /* 这里，动态线程所用的句柄可以临时变量 */
    rt_uint32_t tslice1, tslice2;
    if(argc < 2)
    {
        tslice1 = THREAD_TIMESLICE;
        tslice2 = THREAD_TIMESLICE/2;
    }
    else
    {
        tslice1 = str2dec(argv[1]);
        tslice2 = str2dec(argv[2]);
    }
    /* 创建线程1 */
    tid = rt_thread_create("thread1", 
                            thread_entry, (void*)1,  /* 动态线程的入口参数传递，需要类型转换为(void*) */
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, tslice1); 
    if (tid != RT_NULL) 
    {
        rt_thread_startup(tid); /* 临时变量用于给调度器启动 */
    }


    /* 创建线程2 */
    tid = rt_thread_create("thread2", 
                            thread_entry, (void*)2,
                            THREAD_STACK_SIZE, 
                            THREAD_PRIORITY, tslice2);
    if (tid != RT_NULL)
    {
        rt_thread_startup(tid);
    }
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(timeslice_sample, timeslice sample);

