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
 * 程序清单：互斥锁例程
 *
 * 互斥锁是一种保护共享资源的方法。当一个线程拥有互斥锁的时候，可以保护共享资源不被其他线程破坏。
 * 这里设了两个线程，它们会对两个变量各做+1操作，但对第一个变量+1后到第二个变量+1前，中间都经过一段阻塞。互斥量在这里能保护这两个变量确保同时+1。
 * 为使得演示效果更好，更改了部分官方的源码，添加了提示语句，提示每一步哪个线程在干什么。
 * 增加参数决定是否启用互斥量以做对比（参数为0或者非数字的符号时，会不启用互斥量，不带参数，或参数不为0的数字，则启用）
 */

#include <rtthread.h>

#define THREAD_PRIORITY         8
#define THREAD_TIMESLICE        5

/* 指向互斥量的指针 */
static rt_mutex_t dynamic_mutex = RT_NULL;
static rt_uint8_t number1,number2;

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;

static void numbercheck(void)
{
    if(number1 != number2)
    {
        rt_kprintf("Not pretected. n1 = %d, n2 = %d \n",number1 ,number2);
    }
    else
    {
        rt_kprintf("Pretected. n1 = n2 = %d\n",number1);
    }
}

static void rt_thread_entry1(void *parameter)
{
    rt_uint32_t flag;
    flag = (rt_uint32_t) parameter;
    while(1)
    {
        /* 线程1获取到互斥量后，先后对number1、number2进行加1操作，然后释放互斥量 */
        if(flag)
        {
            rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);   
        }
        rt_kprintf("T1 adding n1 to %d\n",++number1);
        rt_kprintf("T1 blocked.\n");     
        rt_thread_delay(30); /* 在实际应用中，这种延时很常见。但如果不加锁，延时前跟延时后可能就有不同的线程对数值进行了改动*/
        rt_kprintf("T1 adding n2 to %d\n",++number2);
        if(flag)
        {
            rt_mutex_release(dynamic_mutex);
        }
        if(number1 >= 50)
        {
            rt_kprintf("T1 exit.\n");     
            return;
        }
    }
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
static void rt_thread_entry2(void *parameter)
{     
    rt_uint32_t flag;
    flag = (rt_uint32_t) parameter;
    while(1)
    {
    /* 线程2获取到互斥量后，检查number1、number2的值是否相同，相同则表示mutex起到了锁的作用 */
        if(flag)
        {
            rt_mutex_take(dynamic_mutex, RT_WAITING_FOREVER);
        }
        numbercheck();
        /* 线程2也同时两个数加一 */
        rt_kprintf("T2 adding n1 to %d\n",++number1);
        rt_kprintf("T2 blocked.\n");
        rt_thread_delay(10); /* 在实际应用中，这种延时很常见。但如果不加锁，延时前跟延时后可能就有不同的线程对数值进行了改动*/
        rt_kprintf("T2 adding n2 to %d\n",++number2);
        if(flag)
        {
           rt_mutex_release(dynamic_mutex);
        }
        if(number1 >= 50)
        {
            rt_kprintf("T2 exit.\n");     
            return;
        }
      }
}

/* 互斥量示例的初始化 */
int mutex_sample(int argc, char**argv)
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
    
    /* 创建一个动态互斥量 */
    dynamic_mutex = rt_mutex_create("dmutex", RT_IPC_FLAG_FIFO);
    if (dynamic_mutex == RT_NULL)
    {
        rt_kprintf("create dynamic mutex failed.\n");
        return -1;
    }
    number1 = number2 = 0;
    rt_thread_init(&thread1,
                   "thread1",
                   rt_thread_entry1,
                   (void*)flag,
                   &thread1_stack[0],
                   sizeof(thread1_stack), 
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);
    
    rt_thread_init(&thread2,
                   "thread2",
                   rt_thread_entry2,
                   (void*)flag,
                   &thread2_stack[0],
                   sizeof(thread2_stack), 
                   THREAD_PRIORITY-1, THREAD_TIMESLICE);
    rt_thread_startup(&thread2);
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(mutex_sample, mutex sample);
