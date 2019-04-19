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
 * 程序清单：创建/删除、初始化线程等
 *
 * -改写了官方线程。现在这个例子会创建两个线程，都间隔打印有限次。
 * -线程都添加进入与退出提示。增加一个换行避免与finsh命令同行看不清。便于用list_thread等命令查看它了。
 * -动态线程优先级低，静态线程优先级高。它们的线程栈空间也不一样。动态是宏定义并在创建时传入，静态是编译时分配在初始化时并用sizeof获得传入；
 * -可以作为动态静态两种线程的建立的样例。
 */
#include <rtthread.h>

#define THREAD_PRIORITY         25
#define THREAD_STACK_SIZE       512
#define THREAD_TIMESLICE        5

static rt_thread_t tid1 = RT_NULL; /* 动态线程，需要定义控制块指针。栈与控制块本身不用事先定义，用到时动态分配。 */

/* 线程1的入口函数 */
static void thread1_entry(void *parameter)
{
    rt_uint32_t count = 0;
    rt_kprintf("\nthread1 begin\n");
    while (1)
    {
        rt_kprintf("\nthread1 count: %d\n", count ++);
        rt_thread_mdelay(800);
        if(count > 30)
        {
            rt_kprintf("\nthread1 exit\n");
            break;
        }
    }
}

ALIGN(RT_ALIGN_SIZE)
static char thread2_stack[1024]; /* 静态线程需要事先定义线程栈。线程栈空间还需要字节对齐 */
static struct rt_thread thread2; /* 静态线程还需要事先定义控制块 */
/* 线程2入口 */
static void thread2_entry(void *param)
{
    rt_uint32_t count = 0;
    rt_kprintf("\nthread2 begin\n");
    /* 线程2拥有较高的优先级，以抢占线程1而获得执行 */
    for (count = 0; count < 31 ; count++)
    {
        /* 线程2打印计数值 */
        rt_kprintf("\nthread2 count: %d\n", count);
        rt_thread_mdelay(600);
    }
    rt_kprintf("\nthread2 exit\n");
}

/* 线程示例 */
int thread_sample(void)
{
    /* 创建线程1（动态），名称是thread1，入口是thread1_entry。必须获得返回值，即动态分配的控制块的指针。*/
    tid1 = rt_thread_create("thread1",
                            thread1_entry, RT_NULL,
                            THREAD_STACK_SIZE,
                            THREAD_PRIORITY, THREAD_TIMESLICE);
	
    /* 动态时：需要确认分配成功，才能启动这个线程 */
    if (tid1 != RT_NULL)
        rt_thread_startup(tid1);
		
    /* 初始化线程2（静态），名称是thread2，入口是thread2_entry。也会返回控制块的指针，但可以不获取（静态，地址不会变，可用变量寻找到）。 */
    rt_thread_init(&thread2, /* 静态时：控制块的指针也是事先定义并传入；而不是分配后返回。 */
                   "thread2",
                   thread2_entry,
                   RT_NULL,
                   &thread2_stack[0], /* 静态时：线程栈的指针也需要传入。*/
                   sizeof(thread2_stack), /* 静态时：线程栈的大小用sizeof获得并传入；这样如果在变量定义处修改长度，此处也不用再改。*/
                   THREAD_PRIORITY - 1, THREAD_TIMESLICE);
									 rt_thread_startup(&thread2);
    /* 静态时：初始化后无需确认分配是否成功。因为编译时已经分配好。 */
    return 0;
}

/* 导出到 msh 命令列表中 */
MSH_CMD_EXPORT(thread_sample, thread sample);

/* 根据官方视频教程 在片上RAM时，动态静态线程差别不大，一般来说动态的创建更方便；而如果用到外扩RAM，动态线程效率就可能较差。 */
