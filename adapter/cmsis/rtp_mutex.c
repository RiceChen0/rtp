#include "rtp_def.h"
#include "cmsis_os2.h"

rtp_mutex_id rtp_mutex_create(void)
{
    osMutexId_t mutex = NULL;
    mutex = osMutexNew(NULL);

    return (rtp_mutex_id)mutex;
}

rtp_err_t rtp_mutex_lock(rtp_mutex_id mutex)
{
    if (mutex == NULL) {
        return RTP_EINVAL;
    }
    if(osMutexAcquire((osMutexId_t)mutex, osWaitForever) == osOK) {
        return RTP_EOK;
    }
    return RTP_ERROR;
}

rtp_err_t rtp_mutex_unlock(rtp_mutex_id mutex)
{
    if (mutex == NULL) {
        return RTP_EINVAL;
    }
    if(osMutexRelease((osMutexId_t)mutex) == osOK) {
        return RTP_EOK;
    }
    return RTP_ERROR;
}

void rtp_mutex_delete(rtp_mutex_id mutex)
{
    if (mutex == NULL) {
        return;
    }
    osMutexDelete(mutex);
}
