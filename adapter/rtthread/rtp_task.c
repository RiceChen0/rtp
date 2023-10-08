#include "rtp_def.h"
#include "rtthread.h"

rtp_task_id rtp_task_create(rtp_task_func func, void *arg, const struct rtp_task_attr *attr)
{
    rt_thread_t thread = NULL;

    thread = rt_thread_create(attr->name, func, arg, attr->stack_size, attr->priority, 20);
    rt_thread_startup(thread);

    return (rtp_task_id)thread;
}

rtp_err_t rtp_task_suspend(rtp_task_id task)
{
    rt_thread_suspend((rt_thread_t)task);
    return RTP_EOK;
}

rtp_err_t rtp_task_resume(rtp_task_id task)
{
    rt_thread_resume((rt_thread_t)task);
    return RTP_EOK;
}

void rtp_task_delete(rtp_task_id task)
{
    rt_thread_delete((rt_thread_t)task);
}
