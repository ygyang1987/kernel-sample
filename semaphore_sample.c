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
 * 程序清单：信号量例程
 *
 * 该例程创建了一个动态信号量，初始化两个线程，线程1发送信号量，线程2接收并计数。
 * 大幅修改了官方例程，加入线程1与2的延迟，2的延迟比1短以观察实际效果。
 * 原官方代码中无限延迟等待会导致无法触发释放线程2，此处改为有限时间。
 * 并增加了参数输入信号量初值。以进一步观察机制。
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_TIMESLICE        5

/* 指向信号量的指针 */
static rt_sem_t dynamic_sem = RT_NULL;

ALIGN(RT_ALIGN_SIZE)
static char thread1_stack[1024];
static struct rt_thread thread1;
static void rt_thread1_entry(void *parameter)
{
    static rt_uint8_t count = 0;
  
    while(1)
    {
        if(count <= 10)
        {
            count++; /* count每1次就释放1次信号量 */
            rt_kprintf("t1 release a dynamic semaphore.(%d)\n" , count); 
            rt_sem_release(dynamic_sem);                
        }
        else
        {
            rt_kprintf("t1 exit.\n");
            return; /* t1 线程自动结束了。*/
        }
        rt_thread_delay(30); /* 为了与信号量函数统一单位，延迟改用时间片不用ms */
    }
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024];
static struct rt_thread thread2;
static void rt_thread2_entry(void *parameter)
{
    static rt_err_t result;
    static rt_uint8_t number = 0;
    while(1)
    {
        /* 获取到信号量，则执行number自加的操作；若超时则删除信号量与线程 */
        rt_kprintf("t2 taking semaphore... \n");
        result = rt_sem_take(dynamic_sem, 60); //RT_WAITING_FOREVER /* 原官方代码中无限延迟等待会导致无法触发释放线程2 */
        if (result != RT_EOK) 
        {
            rt_kprintf("t2 over time.\n");
            rt_sem_delete(dynamic_sem);
            rt_kprintf("delete semaphore success.\nt2 exit.\n");
            return;
        }
        else
        {
            number++;
            rt_kprintf("t2 success.(%d)\n" ,number);                        
        }
        rt_thread_delay(5);
    }   
}

/* 信号量示例的初始化 */
int semaphore_sample(int argc, char**argv)
{
    extern rt_uint32_t str2dec(char* str);
    rt_uint32_t first_value;
    /* 根据输入参数确定初值 */
    if(argc<1)
    {
        first_value = 0;
    }
    else
    {
        first_value = str2dec(argv[1]);
    }
    /* 创建一个动态信号量，初始值是根据输入参数确定 */

    dynamic_sem = rt_sem_create("dsem", first_value, RT_IPC_FLAG_FIFO);
    if (dynamic_sem == RT_NULL)
    {
        rt_kprintf("create dynamic semaphore failed.\n");
        return -1;
    }
    else
    {
        rt_kprintf("create done. dynamic semaphore value = %d.\n",first_value);
    }

    rt_thread_init(&thread1,
                   "thread1",
                   rt_thread1_entry,
                   RT_NULL,
                   &thread1_stack[0],
                   sizeof(thread1_stack), 
                   THREAD_PRIORITY, THREAD_TIMESLICE);
    rt_thread_startup(&thread1);
                   
    rt_thread_init(&thread2,
                   "thread2",
                   rt_thread2_entry,
                   RT_NULL,
                   &thread2_stack[0],
                   sizeof(thread2_stack), 
                   THREAD_PRIORITY-1, THREAD_TIMESLICE);/* Thread2 优先级比较高 */
    rt_thread_startup(&thread2);

    return 0;
}
/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(semaphore_sample, semaphore sample);

