#include "rtp_def.h"
#include "rtthread.h"

rtp_sem_id rtp_sem_create(uint32_t value)
{
    rt_sem_t sem = NULL;

    sem = rt_sem_create("rtp", value, RT_IPC_FLAG_FIFO);
    return (rtp_sem_id)sem;
}

rtp_err_t rtp_sem_lock(rtp_sem_id sem)
{
    if (sem == NULL) {
        return RTP_EINVAL;
    }
    rt_sem_take((rt_sem_t)sem, RT_WAITING_FOREVER);
    return RTP_ERROR;
}

rtp_err_t rtp_sem_unlock(rtp_sem_id sem)
{
    if (sem == NULL) {
        return RTP_EINVAL;
    }
    rt_sem_release((rt_sem_t)sem);
    return RTP_ERROR;
}

void rtp_sem_delete(rtp_sem_id sem)
{
    rt_sem_delete((rt_sem_t)sem);
}
