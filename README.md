# 问题产生

- 无论是Linux，RTOS，还是Android等开发，我们都会用到多线程编程；但是往往很多人在编程时，都很随意的创建/销毁线程的策略来实现多线程编程；很明显这是不合理的做法，线程的创建/销毁代价是很高的。那么我们要怎么去设计多线程编程呢？？？ 答案：对于长驻的线程，我们可以创建独立的线程去执行。但是非长驻的线程，我们可以通过线程池的方式来处理这些线程。

# 线程池概述

- 线程池，它是一种多线程处理形式，处理过程中将任务添加到队列，然后在创建线程后自动启动这些任务。线程池线程都是后台线程。每个线程都使用默认的堆栈大小，以默认的优先级运行，并处于多线程单元中。如果某个线程在托管代码中空闲（如正在等待某个事件）,则线程池将插入另一个辅助线程来使所有处理器保持繁忙。如果所有线程池线程都始终保持繁忙，但队列中包含挂起的工作，则线程池将在一段时间后创建另一个辅助线程但线程的数目永远不会超过最大值。超过最大值的线程可以排队，但他们要等到其他线程完成后才启动。

- 在一个系统中，线程数过多会带来调度开销，进而影响缓存局部性和整体性能。而线程池维护着多个线程，等待着监督管理者分配可并发执行的任务。这避免了在处理短时间任务时创建与销毁线程的代价。线程池不仅能够保证内核的充分利用，还能防止过分调度。可用线程数量应该取决于可用的并发处理器、处理器内核、内存、网络sockets等的数量。线程数过多会导致额外的线程切换开销。
- 线程的创建-销毁对系统性能影响很大：
  1. 创建太多线程，将会浪费一定的资源，有些线程未被充分使用。
  2. 销毁太多线程，将导致之后浪费时间再次创建它们。
  3. 创建线程太慢，将会导致长时间的等待，性能变差。
  4. 销毁线程太慢，导致其它线程资源饥饿

- 线程池的应用场景：
  1. 单位时间内处理的任务频繁，且任务时间较短；
  2. 对实时性要求较高。如果接收到任务之后再创建线程，可能无法满足实时性的要求，此时必须使用线程池；
  3. 必须经常面对高突发性事件。比如 Web 服务器。如果有足球转播，则服务器将产生巨大冲击，此时使用传统方法，则必须不停的大量创建、销毁线程。此时采用动态线程池可以避免这种情况的发生。

- 线程池的应用例子：
  1. EventBus：它是Android的一个事件发布/订阅轻量级框架。其中事件的异步发布就采用了线程池机制。
  2. Samgr：它是OpenHarmony的一个服务管理组件，解决多服务的管理的策略，减低了线程的创建开销。

- 作者最近在开发的过程中，也遇到多线程编程问题，跨平台，并发任务多，执行周期短。如果按照以往的反复的创建/销毁线程，显然不是一个很好的软件设计。我们需要利用线程池的方式来解决我们问题。

# RTP(Thread Pool)组件

RTP组件，又称线程池组件。是作者编写一个多线程管理组件，特点：

1. 跨平台：它支持任意的RTOS系统，Linux系统。
2. 易移植：该组件默认支持CMSIS和POSIX接口，其他RTOS可以轻易适配兼容。
3. 接口简单：用户操作接口简单，只有三个接口：创建线程池，增加task到线程池，销毁线程池。

## RTP原理

![](https://ricechen0.gitee.io/picture/thread_pool/1.png)

- ① 创建一个线程池，线程池中维护一个Task队列，用于Task任务；理论上：线程池中线程数目至少一个，最多无数个，但是我们要系统能力决定。
- ② 应用层根据业务需求，创建对应Task，Task数目不限制，根据系统资源创建。
- ③ 应用层创建的Task，会被挂在Task队列中。
- ④ 线程池的空闲线程，会检测Task队列中是否为空，如果Task队列不为空，则提取一个Task在线程中执行。

## 核心层实现

tp的提供的接口非常精简：创建线程池，增加任务到线程池，销毁线程池。

1. 创建线程池：

- 接口描述：pf_err_t rtp_create(rtp *pool, const char *name, uint32_t stackSize, uint8_t threadNum);

| **参数**  | **说明**             |
| --------- | -------------------- |
| pool      | 线程池句柄           |
| name      | 线程池中线程名字     |
| stack_size | 线程池中线程的栈大小 |
| thead_num  | 线程池中线程数目     |
| **返回**  | --                   |
| PF_EINVAL | pool无效参数         |
| PF_ERROR  | 创建失败             |
| PF_NOMEM  | 内存不足             |
| PF_EOK    | 创建成功             |

- 接口实现：
  - ①创建task队列增删互斥量：管理task队列的增加及释放的互斥关系，保证增加和释放为同步策略。
  - ②创建task队列状态信号量：当task队列非空则释放信号量，线程池中的线程可以从task队列中获取task执行。
  - ③创建线程池中线程：根据threadNum参数，创建对应的线程数目。

``` C
pf_err_t rtp_create(rtp *pool, const char *name, 
                   uint32_t stack_size, uint8_t thread_num)
{
    int index = 0;
    if(pool == NULL) {
        PF_LOGE("Thread pool handle is NULL");
        return PF_EINVAL;
    }

    if((pool->queue_lock = pf_mutex_create()) == NULL) {
        PF_LOGE("Create thread pool mutex failed");
        return PF_ERROR;
    }
    if((pool->queue_ready = pf_sem_create(0)) == NULL) {
        PF_LOGE("Create thread pool sem failed");
        return PF_ERROR;
    }
    pool->task_queue = NULL;
    pool->thread_num = thread_num;
    pool->wait_task_num = 0;
    pool->threads = (rtp_thread_info *)PF_MALLOC(thread_num * sizeof(rtp_thread_info));
    if(pool->threads == NULL) {
        PF_LOGE("Malloc thread pool info memory failed");
        return PF_ENOMEM;
    }
    for(index = 0; index < thread_num; index++) {
        pool->threads[index].attr.name = (char *)PF_MALLOC(RPF_NAME_LEN);
        if(pool->threads[index].attr.name == NULL) {
            PF_LOGE("Malloc thread name memory failed");
            return PF_ENOMEM;
        }
        snprintf(pool->threads[index].attr.name, RPF_NAME_LEN, "%s%d", name, index);
        pool->threads[index].attr.stack_size = stack_size;
        pool->threads[index].attr.priority = RPF_PRIORITY;
        pool->threads[index].thread_id = pf_task_create(rtp_handler, pool, &pool->threads[index].attr);
        
    }
    return PF_EOK;
}
```

2. 增加任务到线程池：

- 接口描述：pf_err_t rtp_add_task(rtp *pool, task_handle handle, void *argv);

| **参数**  | **说明**             |
| --------- | -------------------- |
| pool      | 线程池句柄           |
| handle    | 线程池中线程名字     |
| argv      | 线程池中线程的栈大小 |
| **返回**  | --                   |
| PF_EINVAL | pool无效参数         |
| PF_NOMEM  | 内存不足             |
| PF_EOK    | 增加task成功         |

- 接口实现：
  - ① 创建一个task句柄，并将注册task函数和函数的入参。
  - ② 获取task队列互斥量，避免增加队列成员时，在释放队列成员。
  - ③ 释放task信号量，通知线程池中的线程可以从task队列中获取task执行

``` C
pf_err_t rtp_add_task(rtp *pool, task_handle handle, void *argv)
{
    rtp_task *new_task = NULL;
    rtp_task *task_lIst = NULL;

    if(pool == NULL) {
        PF_LOGE("Thread pool handle is NULL");
        return PF_EINVAL;
    }

    new_task = (rtp_task *)PF_MALLOC(sizeof(rtp_task));
    if(new_task == NULL) {
        PF_LOGE("Malloc new task handle memory failed");
        return PF_ENOMEM;
    }
    new_task->handle = handle;
    new_task->argv = argv;
    new_task->next = NULL;

    pf_mutex_lock(pool->queue_lock);
    task_lIst = pool->task_queue;
    if(task_lIst == NULL) {
        pool->task_queue = new_task;
    }
    else {
        while(task_lIst->next != NULL) {
            task_lIst = task_lIst->next;
        }
        task_lIst->next = new_task;
    }
    pool->wait_task_num++;
    pf_mutex_unlock(pool->queue_lock);
    pf_sem_unlock(pool->queue_ready);
    return PF_EOK;
}
```

3. 销毁线程池

- 接口描述：pf_err_t rtp_destroy(rtp *pool);

| **参数**  | **说明**     |
| --------- | ------------ |
| pool      | 线程池句柄   |
| **返回**  | --           |
| PF_EINVAL | pool无效参数 |
| PF_EOK    | 销毁成功     |

- 接口实现：
  - ① 删除线程池中所有线程。
  - ② 删除task队列互斥量，task状态信号量。
  - ③ 删除线程池的Task队列。

``` C
pf_err_t rtp_destroy(rtp *pool)
{
    int index = 0;
    rtp_task *head = NULL;

    if(pool == NULL) {
        PF_LOGE("Thread pool handle is NULL");
        return PF_EINVAL;
    }

    for(index = 0; index < pool->thread_num; index++) {
        pf_task_delete(pool->threads[index].thread_id);
        pool->threads[index].thread_id = NULL;
        PF_FREE(pool->threads[index].attr.name);
        pool->threads[index].attr.name = NULL;
    }
    pf_mutex_delete(pool->queue_lock);
    pool->queue_lock = NULL;
    pf_sem_delete(pool->queue_ready);
    pool->queue_ready = NULL;

    PF_FREE(pool->threads);
    pool->threads = NULL;

    while (pool->task_queue != NULL) {
        head = pool->task_queue;
        pool->task_queue = pool->task_queue->next;
        PF_FREE(head);
    }
    pool = NULL;
    return PF_EOK;
}
```

4. 线程池中线程函数

- 接口描述：static void rtp_handler(void *argv)

| **参数** | **说明**   |
| -------- | ---------- |
| argv     | 线程池参数 |

- 接口实现：
  - ① 获取task队列互斥量，避免增加队列成员时，在释放队列成员。
  - ② 当task队列为空时，将阻塞在获取信号量，等待用户增加task时释放信号量。
  - ③ 当task队列不为空，则从task队列中获取task，并执行。
  - ④ 当task执行完，会将对应的task句柄删除。

``` C
static void rtp_handler(void *argv)
{
    rtp *pool = (rtp *)argv;
    rtp_task *task = NULL;

    while(1) {
        pf_mutex_lock(pool->queue_lock);
        while(pool->wait_task_num == 0) {
            pf_mutex_unlock(pool->queue_lock);
            pf_sem_lock(pool->queue_ready);
            pf_mutex_lock(pool->queue_lock);
        }
        task = pool->task_queue;
        pool->wait_task_num--;
        pool->task_queue = task->next;
        pf_mutex_unlock(pool->queue_lock);
        task->handle(task->argv);
        PF_FREE(task);
        task = NULL;
    }
}
```

### TP应用

1. 测试例程：

- 创建一个线程池，线程池中包含3个线程，线程的名字为tp，栈为1024byte。
- 在线程池中创建6个task，其中，task参数为taskId。

``` C
#include "rthread_pool.h"

static rtp pool;

void TestRtpHandle(void *argv)
{
    printf("%s--taskId: %d\r\n", __FUNCTION__, (uint32_t)argv);
}

int main(void)
{
    // ①
    rtp_create(&pool, "tp", 1024, 3);
    // ②
    rtp_add_task(&pool, TestRtpHandle, (void *)1);
    rtp_add_task(&pool, TestRtpHandle, (void *)2);
    rtp_add_task(&pool, TestRtpHandle, (void *)3);
    rtp_add_task(&pool, TestRtpHandle, (void *)4);
    rtp_add_task(&pool, TestRtpHandle, (void *)5);
    rtp_add_task(&pool, TestRtpHandle, (void *)6);

    return 0;
}
```

2. RTOS中的CMSIS运行效果：

![](https://ricechen0.gitee.io/picture/thread_pool/2.png)

3. Linux中POSIX接口运行效果：

![](https://ricechen0.gitee.io/picture/thread_pool/3.png)
