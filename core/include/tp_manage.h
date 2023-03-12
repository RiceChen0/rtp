#ifndef __TP_MANAGE_H__
#define __TP_MANAGE_H__

#include "tp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define TP_THREAD_NAME_LEN        16
#define TP_THREAD_PRIORITY        10

typedef void (*taskHandle)(void *argv);

typedef struct TpTask_ {
    taskHandle handle;
    void *argv;
    struct TpTask_ *next;
}TpTask;

typedef struct {
    TpThreadAttr attr;
    TpThreadId *threadId;
}TpThreadInfo;

typedef struct {
    TpMutexId queueLock;
    TpSemId queueReady;
    TpThreadInfo *threads;
    TpTask *taskQueue;
    uint8_t threadNum;
    uint8_t waitTaskNum;
}Tp;

/**
 * Create thread pool
 * 
 * @param pool      thread pool handle
 * @param name      thread name
 * @param stackSize thread stack size
 * @param threadNum Number of threads in thread pool
 * @return create thread pool result. 
 *         TP_EOK: init success
 *         TP_ERROR: init failed
*/
TpErrCode TpCreate(Tp *pool, const char *name, 
                   uint32_t stackSize, uint8_t threadNum);

/**
 * Adding tasks to the thread pool
 * 
 * @param pool thread pool handle
 * @param handl task handler
 * @param argv task parameter
 * @return add task result. 
 *         TP_EOK: init success
 *         TP_ERROR: init failed
*/
TpErrCode TpAddTask(Tp *pool, taskHandle handle, void *argv);

/**
 * Destroy thread pool
 * 
 * @param pool thread pool handle
 * @return destroy thread pool result. 
 *         TP_EOK: init success
 *         TP_ERROR: init failed
*/
TpErrCode TpDestroy(Tp *pool);

#ifdef __cplusplus
}
#endif

#endif
