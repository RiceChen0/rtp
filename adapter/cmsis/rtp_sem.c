#include "rtp_def.h"
#include "cmsis_os2.h"

rtp_sem_id rtp_sem_create(void)
{
    osSemaphoreId_t sem = NULL;
    sem = osSemaphoreNew(1, value, NULL);

    return (rtp_sem_id)sem;
}

rtp_err_t rtp_sem_lock(rtp_sem_id sem)
{
    if (sem == NULL) {
        return RTP_EINVAL;
    }
    if(osSemaphoreAcquire((osSemaphoreId_t)sem, osWaitForever) == osOK) {
        return RTP_EOK;
    }
    return RTP_ERROR;
}

rtp_err_t rtp_sem_unlock(rtp_sem_id sem)
{
    if (sem == NULL) {
        return RTP_EINVAL;
    }
    if(osSemaphoreRelease((osSemaphoreId_t)sem) == osOK) {
        return RTP_EOK;
    }
    return RTP_ERROR;
}

void rtp_sem_delete(rtp_sem_id sem)
{
    if (sem == NULL) {
        return;
    }
    osSemaphoreDelete((osSemaphoreId_t)sem);
}
