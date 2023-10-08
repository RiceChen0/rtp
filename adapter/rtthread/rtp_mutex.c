#include "rtp_def.h"
#include "rtthread.h"

rtp_mutex_id rtp_mutex_create(void)
{
    rt_mutex_t mutex = NULL;

    mutex = rt_mutex_create("rtp", RT_IPC_FLAG_FIFO);
    return (rtp_mutex_id)mutex;
}

rtp_err_t rtp_mutex_lock(rtp_mutex_id mutex)
{
    if (mutex == NULL) {
        return RTP_EINVAL;
    }
    rt_mutex_take((rt_mutex_t)mutex, RT_WAITING_FOREVER);
    return RTP_ERROR;
}

rtp_err_t rtp_mutex_unlock(rtp_mutex_id mutex)
{
    if (mutex == NULL) {
        return RTP_EINVAL;
    }
    rt_mutex_release((rt_mutex_t)mutex);
    return RTP_ERROR;
}

void rtp_mutex_delete(rtp_mutex_id mutex)
{
    rt_mutex_delete((rt_mutex_t)mutex);
}
