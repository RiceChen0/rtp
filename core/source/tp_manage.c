#include "tp_manage.h"

static void *TpThreadHandler(void *argv);

TpErrCode TpCreate(Tp *pool, const char *name, 
                   uint32_t stackSize, uint8_t threadNum)
{
    int index = 0;
    if(pool == NULL) {
        TP_LOGE("Thread pool handle is NULL");
        return TP_EINVAL;
    }

    if((pool->queueLock = TpMutexCreate()) == NULL) {
        TP_LOGE("Create thread pool mutex failed");
        return TP_ERROR;
    }
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

TpErrCode TpAddTask(Tp *pool, taskHandle handle, void *argv)
{
    TpTask *newTask = NULL;
    TpTask *taskLIst = NULL;

    if(pool == NULL) {
        TP_LOGE("Thread pool handle is NULL");
        return TP_EINVAL;
    }

    newTask = (TpTask *)TP_MALLOC(sizeof(TpTask));
    if(newTask == NULL) {
        TP_LOGE("Malloc new task handle memory failed");
        return TP_ENOMEM;
    }
    newTask->handle = handle;
    newTask->argv = argv;
    newTask->next = NULL;

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
    TpSemRelease(pool->queueReady);
    return TP_EOK;
}

TpErrCode TpDestroy(Tp *pool)
{
    int index = 0;
    TpTask *head = NULL;

    if(pool == NULL) {
        TP_LOGE("Thread pool handle is NULL");
        return TP_EINVAL;
    }

    for(index = 0; index < pool->threadNum; index++) {
        TpThreadDelete(pool->threads[index].threadId);
        pool->threads[index].threadId = NULL;
        TP_FREE(pool->threads[index].attr.name);
        pool->threads[index].attr.name = NULL;
    }
    TpMutexDelete(pool->queueLock);
    pool->queueLock = NULL;
    TpSemDelete(pool->queueReady);
    pool->queueReady = NULL;

    TP_FREE(pool->threads);
    pool->threads = NULL;

    while (pool->taskQueue != NULL) {
        head = pool->taskQueue;
        pool->taskQueue = pool->taskQueue->next;
        TP_FREE(head);
    }
    pool = NULL;
    return TP_EOK;
}

static void *TpThreadHandler(void *argv)
{
    Tp *pool = (Tp *)argv;
    TpTask *task = NULL;

    while(1) {
        TpMutexLock(pool->queueLock);
        while(pool->waitTaskNum == 0) {
            TpMutexUnlock(pool->queueLock);
            TpSemAcquire(pool->queueReady);
            TpMutexLock(pool->queueLock);
        }
        task = pool->taskQueue;
        pool->waitTaskNum--;
        pool->taskQueue = task->next;
        TpMutexUnlock(pool->queueLock);
        task->handle(task->argv);
        TP_FREE(task);
        task = NULL;
    }
}
