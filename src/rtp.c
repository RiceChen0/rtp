#include "rtp.h"

static void rtp_handler(void *argv);

rtp_err_t rtp_create(rtp *pool, const char *name, 
                   uint32_t stack_size, uint8_t thread_num)
{
    int index = 0;
    if(pool == NULL) {
        RTP_LOGE("Thread pool handle is NULL");
        return RTP_EINVAL;
    }

    if((pool->queue_lock = rtp_mutex_create()) == NULL) {
        RTP_LOGE("Create thread pool mutex failed");
        return RTP_ERROR;
    }
    if((pool->queue_ready = rtp_sem_create(0)) == NULL) {
        RTP_LOGE("Create thread pool sem failed");
        return RTP_ERROR;
    }
    pool->task_queue = NULL;
    pool->thread_num = thread_num;
    pool->wait_task_num = 0;
    pool->threads = (rtp_thread_info *)RTP_MALLOC(thread_num * sizeof(rtp_thread_info));
    if(pool->threads == NULL) {
        RTP_LOGE("Malloc thread pool info memory failed");
        return RTP_ENOMEM;
    }
    for(index = 0; index < thread_num; index++) {
        pool->threads[index].attr.name = (char *)RTP_MALLOC(RTP_NAME_LEN);
        if(pool->threads[index].attr.name == NULL) {
            RTP_LOGE("Malloc thread name memory failed");
            return RTP_ENOMEM;
        }
        snprintf(pool->threads[index].attr.name, RTP_NAME_LEN, "%s%d", name, index);
        pool->threads[index].attr.stack_size = stack_size;
        pool->threads[index].attr.priority = RTP_PRIORITY;
        pool->threads[index].thread_id = rtp_task_create(rtp_handler, pool, &pool->threads[index].attr);
        
    }
    return RTP_EOK;
}

rtp_err_t rtp_add_task(rtp *pool, task_handle handle, void *argv)
{
    rtp_task *new_task = NULL;
    rtp_task *task_lIst = NULL;

    if(pool == NULL) {
        RTP_LOGE("Thread pool handle is NULL");
        return RTP_EINVAL;
    }

    new_task = (rtp_task *)RTP_MALLOC(sizeof(rtp_task));
    if(new_task == NULL) {
        RTP_LOGE("Malloc new task handle memory failed");
        return RTP_ENOMEM;
    }
    new_task->handle = handle;
    new_task->argv = argv;
    new_task->next = NULL;

    rtp_mutex_lock(pool->queue_lock);
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
    rtp_mutex_unlock(pool->queue_lock);
    rtp_sem_unlock(pool->queue_ready);
    return RTP_EOK;
}

void rtp_suspend(rtp *pool)
{
    int index = 0;
    uint8_t thread_num;

    thread_num = pool->thread_num;
    for(index = 0; index < thread_num; index++) {
        rtp_task_suspend(pool->threads[index].thread_id);
    }
}

void rtp_resume(rtp *pool)
{
    int index = 0;
    uint8_t thread_num;

    thread_num = pool->thread_num;
    for(index = 0; index < thread_num; index++) {
        rtp_task_resume(pool->threads[index].thread_id);
    }
}

int rtp_wait_task_num(rtp *pool)
{
    return pool->wait_task_num;
}

rtp_err_t rtp_destroy(rtp *pool)
{
    int index = 0;
    rtp_task *head = NULL;

    if(pool == NULL) {
        RTP_LOGE("Thread pool handle is NULL");
        return RTP_EINVAL;
    }

    for(index = 0; index < pool->thread_num; index++) {
        rtp_task_delete(pool->threads[index].thread_id);
        pool->threads[index].thread_id = NULL;
        RTP_FREE(pool->threads[index].attr.name);
        pool->threads[index].attr.name = NULL;
    }
    rtp_mutex_delete(pool->queue_lock);
    pool->queue_lock = NULL;
    rtp_sem_delete(pool->queue_ready);
    pool->queue_ready = NULL;

    RTP_FREE(pool->threads);
    pool->threads = NULL;

    while (pool->task_queue != NULL) {
        head = pool->task_queue;
        pool->task_queue = pool->task_queue->next;
        RTP_FREE(head);
    }
    pool = NULL;
    return RTP_EOK;
}

static void rtp_handler(void *argv)
{
    rtp *pool = (rtp *)argv;
    rtp_task *task = NULL;

    while(1) {
        rtp_mutex_lock(pool->queue_lock);
        while(pool->wait_task_num == 0) {
            rtp_mutex_unlock(pool->queue_lock);
            rtp_sem_lock(pool->queue_ready);
            rtp_mutex_lock(pool->queue_lock);
        }
        task = pool->task_queue;
        pool->wait_task_num--;
        pool->task_queue = task->next;
        rtp_mutex_unlock(pool->queue_lock);
        task->handle(task->argv);
        RTP_FREE(task);
        task = NULL;
    }
}
