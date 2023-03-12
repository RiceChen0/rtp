#include "tp_def.h"
#include "cmsis_os2.h"

TpMutexId TpMutexCreate(void)
{
    osMutexId_t mutex = NULL;
    mutex = osMutexNew(NULL);

    return (TpMutexId)mutex;
}

TpErrCode TpMutexLock(TpMutexId mutex)
{
    if (mutex == NULL) {
        return TP_EINVAL;
    }
    if(osMutexAcquire((osMutexId_t)mutex, osWaitForever) == osOK) {
        return TP_EOK;
    }
    return TP_ERROR;
}

TpErrCode TpMutexUnlock(TpMutexId mutex)
{
    if (mutex == NULL) {
        return TP_EINVAL;
    }
    if(osMutexRelease((osMutexId_t)mutex) == osOK) {
        return TP_EOK;
    }
    return TP_ERROR;
}

void TpMutexDelete(TpMutexId mutex)
{
    if (mutex == NULL) {
        return;
    }
    osMutexDelete(mutex);
}
