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

# TP(Thread Pool)组件

TP组件，又称线程池组件。是作者编写一个多线程管理组件，特点：

1. 跨平台：它支持任意的RTOS系统，Linux系统。
2. 易移植：该组件默认支持CMSIS和POSIX接口，其他RTOS可以轻易适配兼容。
3. 接口简单：用户操作接口简单，只有三个接口：创建线程池，增加task到线程池，销毁线程池。

## TP原理

![](https://ricechen0.gitee.io/picture/thread_pool/1.png)

- ① 创建一个线程池，线程池中维护一个Task队列，用于Task任务；理论上：线程池中线程数目至少一个，最多无数个，但是我们要系统能力决定。
- ② 应用层根据业务需求，创建对应Task，Task数目不限制，根据系统资源创建。
- ③ 应用层创建的Task，会被挂在Task队列中。
- ④ 线程池的空闲线程，会检测Task队列中是否为空，如果Task队列不为空，则提取一个Task在线程中执行。

## TP实现

### 适配层实现

为了实现跨平台，需要将差异性接口抽象出来，我们整个组件需要抽象几个内容：①日志接口；②内存管理接口；③ 线程接口；④互斥量接口；⑤信号量接口。以CMSIS接口为例的实现：

1. 错误码：提供了四种错误码：无错误，错误，内存不足，无效参数。

``` C
typedef enum{
    TP_EOK = 0,         // There is no error
    TP_ERROR,           // A generic error happens
    TP_ENOMEM,          // No memory
    TP_EINVAL,          // Invalid argument
} TpErrCode;
```

2. 日志接口适配：

- 需修改宏定义：TP_PRINT；
- 支持三个等级日志打印：错误信息日志，运行信息日志，调试信息日志的打印。并且支持带颜色。

``` C
#define TP_PRINT        printf

#define TP_LOGE(...)    TP_PRINT("\033[31;22m[E/TP](%s:%d) ", __FUNCTION__, __LINE__);  \
                        TP_PRINT(__VA_ARGS__);                                          \
                        TP_PRINT("\033[0m\n")
#define TP_LOGI(...)    TP_PRINT("\033[32;22m[I/TP](%s:%d) ", __FUNCTION__, __LINE__);  \
                        TP_PRINT(__VA_ARGS__);                                          \
                        TP_PRINT("\033[0m\n")
#define TP_LOGD(...)    TP_PRINT("[D/TP](%s:%d) ", __FUNCTION__, __LINE__);             \
                        TP_PRINT(__VA_ARGS__);                                          \
                        TP_PRINT("\n")
```

3. 内存接口：只需适配申请内存和释放内存宏定义

``` C
#define TP_MALLOC       malloc     
#define TP_FREE         free
```

4. 线程接口：

``` C
// tp_def.h
typedef void *TpThreadId;

typedef void *(*tpThreadFunc)(void *argv);

typedef struct {
    char *name;
    uint32_t stackSize;
    uint32_t priority : 8;
    uint32_t reserver : 24;
} TpThreadAttr;

TpThreadId TpThreadCreate(tpThreadFunc func, void *argv, const TpThreadAttr *attr);
void TpThreadDelete(TpThreadId thread);
```

- 创建线程: TpThreadId TpThreadCreate(tpThreadFunc func, void *argv, const TpThreadAttr *attr);

| **参数** | **说明**                         |
| -------- | -------------------------------- |
| func     | 线程入口函数                     |
| argv     | 线程入口函数参数                 |
| attr     | 线程属性：线程名，栈空间，优先级 |
| **返回** | --                               |
| NULL     | 创建失败                         |
| 线程句柄 | 创建成功                         |

- 删除线程：void TpThreadDelete(TpThreadId thread);

| **参数** | **说明** |
| -------- | -------- |
| thread   | 线程句柄 |

- CMSIS适配：

``` C
// tp_threa_adapter.c
#include "tp_def.h"
#include "cmsis_os2.h"

TpThreadId TpThreadCreate(tpThreadFunc func, void *argv, const TpThreadAttr *attr)
{
    osThreadId_t thread = NULL;
    osThreadAttr_t taskAttr = {
        .name = attr->name,
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0,
        .stack_mem = NULL,
        .stack_size = attr->stackSize,
        .priority = (osPriority_t)attr->priority,
        .tz_module = 0,
        .reserved = 0,
    };

    thread = osThreadNew((osThreadFunc_t)func, argv, &taskAttr);
    return (TpThreadId)thread;
}

void TpThreadDelete(TpThreadId thread)
{
    if(thread != NULL) {
        osThreadTerminate(thread);
    }
}
```

5. 互斥量接口：

``` C
// tp_def.h
typedef void *TpMutexId;

TpMutexId TpMutexCreate(void);
TpErrCode TpMutexLock(TpMutexId mutex);
TpErrCode TpMutexUnlock(TpMutexId mutex);
void TpMutexDelete(TpMutexId mutex);
```

- 创建互斥量：TpMutexId TpMutexCreate(void);

| **参数**   | **说明** |
| ---------- | -------- |
| **返回**   | --       |
| NULL       | 创建失败 |
| 互斥量句柄 | 创建成功 |

- 获取互斥量：TpErrCode TpMutexLock(TpMutexId mutex);

| **参数**  | **说明**       |
| --------- | -------------- |
| mutex     | 互斥量句柄     |
| **返回**  | --             |
| TP_EINVAL | mutex无效参数  |
| TP_ERROR  | 获取互斥量失败 |
| TP_EOK    | 成功获取互斥量 |

- 释放互斥量：TpErrCode TpMutexUnlock(TpMutexId mutex);

| **参数**  | **说明**       |
| --------- | -------------- |
| mutex     | 互斥量句柄     |
| **返回**  | --             |
| TP_EINVAL | mutex无效参数  |
| TP_ERROR  | 释放互斥量失败 |
| TP_EOK    | 成功释放互斥量 |

- 删除互斥量：void TpMutexDelete(TpMutexId mutex);

| **参数** | **说明**   |
| -------- | ---------- |
| mutex    | 互斥量句柄 |

- CMSIS适配：

``` C
// tp_mutex_adapter.c
#include "tp_def.h"
#include "cmsis_os2.h"

TpMutexId TpMutexCreate(void)
{
    osMutexId_t mutex = NULL;
    mutex = osMutexNew(NULL);

    return (TpMutexId)mutex;
}

TpErrCode TpMutexLock(TpMutexId mutex)
{
    if (mutex == NULL) {
        return TP_EINVAL;
    }
    if(osMutexAcquire((osMutexId_t)mutex, osWaitForever) == osOK) {
        return TP_EOK;
    }
    return TP_ERROR;
}

TpErrCode TpMutexUnlock(TpMutexId mutex)
{
    if (mutex == NULL) {
        return TP_EINVAL;
    }
    if(osMutexRelease((osMutexId_t)mutex) == osOK) {
        return TP_EOK;
    }
    return TP_ERROR;
}

void TpMutexDelete(TpMutexId mutex)
{
    if (mutex == NULL) {
        return;
    }
    osMutexDelete(mutex);
}
```

6. 信号量接口：

``` C
// tp_def.h
typedef void *TpSemId;

TpSemId TpSemCreate(uint32_t value);
TpErrCode TpSemAcquire(TpSemId sem);
TpErrCode TpSemRelease(TpSemId sem);
void TpSemDelete(TpSemId sem);
```

- 创建信号量：TpSemId TpSemCreate(uint32_t value);

| **参数**   | **说明** |
| ---------- | -------- |
| **返回**   | --       |
| NULL       | 创建失败 |
| 信号量句柄 | 创建成功 |

- 获取信号量：TpErrCode TpSemAcquire(TpSemId sem);

| **参数**  | **说明**       |
| --------- | -------------- |
| sem       | 信号量句柄     |
| **返回**  | --             |
| TP_EINVAL | sem无效参数    |
| TP_ERROR  | 获取信号量失败 |
| TP_EOK    | 成功获取信号量 |

- 释放信号量：TpErrCode TpSemRelease(TpSemId sem);

| **参数**  | **说明**       |
| --------- | -------------- |
| sem       | 信号量句柄     |
| **返回**  | --             |
| TP_EINVAL | 信号量无效参数 |
| TP_ERROR  | 释放信号量失败 |
| TP_EOK    | 成功释放信号量 |

- 删除信号量：void TpSemDelete(TpSemId sem);

| **参数** | **说明**   |
| -------- | ---------- |
| sem      | 信号量句柄 |

- CMSIS适配：

``` C
// tp_sem_adapter.c
#include "tp_def.h"
#include "cmsis_os2.h"

TpSemId TpSemCreate(uint32_t value)
{
    osSemaphoreId_t sem = NULL;
    sem = osSemaphoreNew(1, value, NULL);

    return (TpSemId)sem;
}

TpErrCode TpSemAcquire(TpSemId sem)
{
    if (sem == NULL) {
        return TP_EINVAL;
    }
    if(osSemaphoreAcquire((osSemaphoreId_t)sem, osWaitForever) != osOK) {
        return TP_ERROR;
    }
    return TP_EOK;
}

TpErrCode TpSemRelease(TpSemId sem)
{
    if (sem == NULL) {
        return TP_EINVAL;
    }
    if(osSemaphoreRelease((osSemaphoreId_t)sem) != osOK) {
        return TP_ERROR;
    }
    return TP_EOK;
}

void TpSemDelete(TpSemId sem)
{
    if (sem == NULL) {
        return;
    }
    osSemaphoreDelete((osSemaphoreId_t)sem);
}
```

### 核心层实现

tp的提供的接口非常精简：创建线程池，增加任务到线程池，销毁线程池。

1. 创建线程池：

- 接口描述：TpErrCode TpCreate(Tp *pool, const char *name, uint32_t stackSize, uint8_t threadNum);

| **参数**  | **说明**             |
| --------- | -------------------- |
| pool      | 线程池句柄           |
| name      | 线程池中线程名字     |
| stackSize | 线程池中线程的栈大小 |
| theadNum  | 线程池中线程数目     |
| **返回**  | --                   |
| TP_EINVAL | pool无效参数         |
| TP_ERROR  | 创建失败             |
| TP_NOMEM  | 内存不足             |
| TP_EOK    | 创建成功             |

- 接口实现：
  - ①创建task队列增删互斥量：管理task队列的增加及释放的互斥关系，保证增加和释放为同步策略。
  - ②创建task队列状态信号量：当task队列非空则释放信号量，线程池中的线程可以从task队列中获取task执行。
  - ③创建线程池中线程：根据threadNum参数，创建对应的线程数目。

``` C
TpErrCode TpCreate(Tp *pool, const char *name, 
                   uint32_t stackSize, uint8_t threadNum)
{
    int index = 0;
    if(pool == NULL) {
        TP_LOGE("Thread pool handle is NULL");
        return TP_EINVAL;
    }
    // ①
    if((pool->queueLock = TpMutexCreate()) == NULL) {
        TP_LOGE("Create thread pool mutex failed");
        return TP_ERROR;
    }
    // ②
    if((pool->queueReady = TpSemCreate(0)) == NULL) {
        TP_LOGE("Create thread pool sem failed");
        return TP_ERROR;
    }
    pool->taskQueue = NULL;
    pool->threadNum = threadNum;
    pool->waitTaskNum = 0;
    pool->threads = (TpThreadInfo *)TP_MALLOC(threadNum * sizeof(TpThreadInfo));
    if(pool->threads == NULL) {
        TP_LOGE("Malloc thread pool info memory failed");
        return TP_ENOMEM;
    }
    // ③
    for(index = 0; index < threadNum; index++) {
        pool->threads[index].attr.name = (char *)TP_MALLOC(TP_THREAD_NAME_LEN);
        if(pool->threads[index].attr.name == NULL) {
            TP_LOGE("Malloc thread name memory failed");
            return TP_ENOMEM;
        }
        snprintf(pool->threads[index].attr.name, TP_THREAD_NAME_LEN, "%s%d", name, index);
        pool->threads[index].attr.stackSize = stackSize;
        pool->threads[index].attr.priority = TP_THREAD_PRIORITY;
        pool->threads[index].threadId = TpThreadCreate(TpThreadHandler, pool, &pool->threads[index].attr);
        
    }
    return TP_EOK;
}
```

2. 增加任务到线程池：

- 接口描述：TpErrCode TpAddTask(Tp *pool, taskHandle handle, void *argv);

| **参数**  | **说明**             |
| --------- | -------------------- |
| pool      | 线程池句柄           |
| handle    | 线程池中线程名字     |
| argv      | 线程池中线程的栈大小 |
| **返回**  | --                   |
| TP_EINVAL | pool无效参数         |
| TP_NOMEM  | 内存不足             |
| TP_EOK    | 增加task成功         |

- 接口实现：
  - ① 创建一个task句柄，并将注册task函数和函数的入参。
  - ② 获取task队列互斥量，避免增加队列成员时，在释放队列成员。
  - ③ 释放task信号量，通知线程池中的线程可以从task队列中获取task执行

``` C
TpErrCode TpAddTask(Tp *pool, taskHandle handle, void *argv)
{
    TpTask *newTask = NULL;
    TpTask *taskLIst = NULL;

    if(pool == NULL) {
        TP_LOGE("Thread pool handle is NULL");
        return TP_EINVAL;
    }
    // ①
    newTask = (TpTask *)TP_MALLOC(sizeof(TpTask));
    if(newTask == NULL) {
        TP_LOGE("Malloc new task handle memory failed");
        return TP_ENOMEM;
    }
    newTask->handle = handle;
    newTask->argv = argv;
    newTask->next = NULL;
    // ②
    TpMutexLock(pool->queueLock);
    taskLIst = pool->taskQueue;
    if(taskLIst == NULL) {
        pool->taskQueue = newTask;
    }
    else {
        while(taskLIst->next != NULL) {
            taskLIst = taskLIst->next;
        }
        taskLIst->next = newTask;
    }
    pool->waitTaskNum++;
    TpMutexUnlock(pool->queueLock);
    // ③
    TpSemRelease(pool->queueReady);
    return TP_EOK;
}
```

3. 销毁线程池

- 接口描述：TpErrCode TpDestroy(Tp *pool);

| **参数**  | **说明**     |
| --------- | ------------ |
| pool      | 线程池句柄   |
| **返回**  | --           |
| TP_EINVAL | pool无效参数 |
| TP_EOK    | 销毁成功     |

- 接口实现：
  - ① 删除线程池中所有线程。
  - ② 删除task队列互斥量，task状态信号量。
  - ③ 删除线程池的Task队列。

``` C
TpErrCode TpDestroy(Tp *pool)
{
    int index = 0;
    TpTask *head = NULL;

    if(pool == NULL) {
        TP_LOGE("Thread pool handle is NULL");
        return TP_EINVAL;
    }
    // ①
    for(index = 0; index < pool->threadNum; index++) {
        TpThreadDelete(pool->threads[index].threadId);
        pool->threads[index].threadId = NULL;
        TP_FREE(pool->threads[index].attr.name);
        pool->threads[index].attr.name = NULL;
    }
    // ②
    TpMutexDelete(pool->queueLock);
    pool->queueLock = NULL;
    TpSemDelete(pool->queueReady);
    pool->queueReady = NULL;

    TP_FREE(pool->threads);
    pool->threads = NULL;
    // ③
    while (pool->taskQueue != NULL) {
        head = pool->taskQueue;
        pool->taskQueue = pool->taskQueue->next;
        TP_FREE(head);
    }
    pool = NULL;
    return TP_EOK;
}
```

4. 线程池中线程函数

- 接口描述：static void *TpThreadHandler(void *argv)

| **参数** | **说明**   |
| -------- | ---------- |
| argv     | 线程池参数 |

- 接口实现：
  - ① 获取task队列互斥量，避免增加队列成员时，在释放队列成员。
  - ② 当task队列为空时，将阻塞在获取信号量，等待用户增加task时释放信号量。
  - ③ 当task队列不为空，则从task队列中获取task，并执行。
  - ④ 当task执行完，会将对应的task句柄删除。

``` C
static void *TpThreadHandler(void *argv)
{
    Tp *pool = (Tp *)argv;
    TpTask *task = NULL;

    while(1) {
        // ①
        TpMutexLock(pool->queueLock);
        // ②
        while(pool->waitTaskNum == 0) {
            TpMutexUnlock(pool->queueLock);
            TpSemAcquire(pool->queueReady);
            TpMutexLock(pool->queueLock);
        }
        // ③
        task = pool->taskQueue;
        pool->waitTaskNum--;
        pool->taskQueue = task->next;
        TpMutexUnlock(pool->queueLock);
        task->handle(task->argv);
        // ④
        TP_FREE(task);
        task = NULL;
    }
}
```

### TP应用

1. 测试例程：

- 创建一个线程池，线程池中包含3个线程，线程的名字为tp，栈为1024byte。
- 在线程池中创建6个task，其中，task参数为taskId。

``` C
#include "tp_manage.h"

Tp pool;
void TestTaskHandle(void *argv)
{
    printf("%s--taskId: %d\r\n", __FUNCTION__, (uint32_t)argv);
}

int main(void)
{
    // ①
    TpCreate(&pool, "tp", 1024, 3);
    // ②
    TpAddTask(&pool, TestTaskHandle, (void *)1);
    TpAddTask(&pool, TestTaskHandle, (void *)2);
    TpAddTask(&pool, TestTaskHandle, (void *)3);
    TpAddTask(&pool, TestTaskHandle, (void *)4);
    TpAddTask(&pool, TestTaskHandle, (void *)5);
    TpAddTask(&pool, TestTaskHandle, (void *)6);

    return 0;
}
```

2. RTOS中的CMSIS运行效果：

![](https://ricechen0.gitee.io/picture/thread_pool/2.png)

3. Linux中POSIX接口运行效果：

![](https://ricechen0.gitee.io/picture/thread_pool/3.png)

## 总结

1. 线程池是多线程的一个编程方式，它避免了线程的创建和销毁的开销，提高了系统的性能。
2. 增加到线程池中的任务是非长驻的，不能存在死循环，否则她会一直持有线程池中的某一个线程。
3. TP线程池组件的开发仓库链接 [TP组件](https://gitee.com/RiceChen0/tp.git)


<br/>关注微信公众号『Rice嵌入式开发技术分享』，后台回复“微信”添加作者微信，备注”入群“，便可邀请进入技术交流群。

![](https://ricechen0.gitee.io/picture/logo/logo_.jpg)