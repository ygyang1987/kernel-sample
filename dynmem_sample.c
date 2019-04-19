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
 * 程序清单：动态内存管理例程
 *
 * 这个程序会创建一个动态的线程，这个线程会动态申请内存并释放
 * 每次申请双倍的内存，当申请不到的时候就结束
 * -改写：通过输入参数确定第一次申请空间的大小。
 */
#include <rtthread.h>

#define THREAD_PRIORITY      25
#define THREAD_STACK_SIZE    512
#define THREAD_TIMESLICE     5

/* 线程入口 */
static void thread1_entry(void *parameter)
{
    rt_uint32_t malloc_size;
    rt_uint32_t i;
    char *ptr = RT_NULL; /* 内存块的指针 */

    malloc_size = *((rt_uint32_t*) parameter);
    for (i = 0; ; i++)
    {
        ptr = rt_malloc(malloc_size); 
        if (ptr != RT_NULL) /* 如果分配失败会返回RT_NULL，成功就不会 */
        {
            rt_kprintf("get memory :%d byte\n", (malloc_size));
            rt_free(ptr);/* 释放内存块 */
            rt_kprintf("free memory :%d byte\n", (malloc_size));
            ptr = RT_NULL;

            if (malloc_size>(RT_UINT32_MAX>>1))
            {
                return;
            }
            else
            {
                malloc_size <<=1;
            }
        }
        else
        {
            rt_kprintf("try to get %d byte memory failed!\n", (malloc_size));
            return;
        }
    }
}

static int dynmem_sample(int argc, char**argv)
{
    extern rt_uint32_t str2dec(char* str);
    static rt_uint32_t temp;
    rt_thread_t tid;

    if (argc < 1)
    {
        temp = 1;
    }
    else 
    {
        temp = str2dec(argv[1]);
    }

    if(temp == 0)
    {
        temp = 1;
    }

    /* 创建线程1 */
    tid = rt_thread_create("thread1",
                           thread1_entry, &temp,
                           THREAD_STACK_SIZE,
                           THREAD_PRIORITY,
                           THREAD_TIMESLICE);
    if (tid != RT_NULL)
        rt_thread_startup(tid);

    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(dynmem_sample, dynmem sample);
