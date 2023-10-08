#ifndef __RTP_H__
#define __RTP_H__

#include "rtp_def.h"

#ifdef __cplusplus
extern "C" {
#endif

#define RTP_NAME_LEN        16
#define RTP_PRIORITY        10

typedef void (*task_handle)(void *argv);

typedef struct rtp_task_ {
    task_handle handle;
    void *argv;
    struct rtp_task_ *next;
}rtp_task;

typedef struct {
    struct rtp_task_attr attr;
    rtp_task_id *thread_id;
}rtp_thread_info;

typedef struct {
    rtp_mutex_id queue_lock;
    rtp_sem_id queue_ready;
    rtp_thread_info *threads;
    rtp_task *task_queue;
    uint8_t thread_num;
    uint8_t wait_task_num;
}rtp;

/**
 * Create thread pool
 * 
 * @param pool      thread pool handle
 * @param name      thread name
 * @param stackSize thread stack size
 * @param threadNum Number of threads in thread pool
 * @return create thread pool result. 
 *         RTP_EOK: init success
 *         RTP_ERROR: init failed
*/
rtp_err_t rtp_create(rtp *pool, const char *name, 
                   uint32_t stackSize, uint8_t threadNum);

/**
 * Adding tasks to the thread pool
 * 
 * @param pool thread pool handle
 * @param handl task handler
 * @param argv task parameter
 * @return add task result. 
 *         RTP_EOK: init success
 *         RTP_ERROR: init failed
*/
rtp_err_t rtp_add_task(rtp *pool, task_handle handle, void *argv);

/**
 * Suspend all threads in the thread pool
 * 
 * @param pool thread pool handle
*/
void rtp_suspend(rtp *pool);

/**
 * Resume all threads in the thread pool
 * 
 * @param pool thread pool handle
*/
void rtp_resume(rtp *pool);

/**
 * Get the number of tasks in the thread pool
 * 
 * @param pool thread pool handle
 * @return task number.
*/
int rtp_wait_task_num(rtp *pool);

/**
 * Destroy thread pool
 * 
 * @param pool thread pool handle
 * @return destroy thread pool result. 
 *         RTP_EOK: init success
 *         RTP_ERROR: init failed
*/
rtp_err_t rtp_destroy(rtp *pool);

#ifdef __cplusplus
}
#endif

#endif
