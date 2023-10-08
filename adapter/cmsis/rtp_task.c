#include "rtp_def.h"
#include "cmsis_os2.h"

rtp_task_id rtp_task_create(rtp_task_func func, void *arg, const struct rtp_task_attr *attr)
{
    osThreadId_t thread = NULL;
    osThreadAttr_t task_attr = {
        .name = attr->name,
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0,
        .stack_mem = NULL,
        .stack_size = attr->stack_size,
        .priority = (osPriority_t)attr->priority,
        .tz_module = 0,
        .reserved = 0,
    };

    thread = osThreadNew((osThreadFunc_t)func, arg, &task_attr);
    return (rtp_task_id)thread;
}

rtp_err_t rtp_task_suspend(rtp_task_id task)
{
    osThreadSuspend((osThreadId_t)task);
    return RTP_EOK;
}

rtp_err_t rtp_task_resume(rtp_task_id task)
{
    osThreadResume((osThreadId_t)task);
    return RTP_EOK;
}

void rtp_task_delete(rtp_task_id task)
{
    if(task != NULL) {
        osThreadTerminate((osThreadId_t)task);
    }
}
