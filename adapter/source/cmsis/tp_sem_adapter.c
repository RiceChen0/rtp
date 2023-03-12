#include "tp_def.h"
#include "cmsis_os2.h"

TpSemId TpSemCreate(uint32_t value)
{
    osSemaphoreId_t sem = NULL;
    sem = osSemaphoreNew(1, value, NULL);

    return (TpSemId)sem;
}

TpErrCode TpSemAcquire(TpSemId sem)
{
    if (sem == NULL) {
        return TP_EINVAL;
    }
    if(osSemaphoreAcquire((osSemaphoreId_t)sem, osWaitForever) != osOK) {
        return TP_ERROR;
    }
    return TP_EOK;
}

TpErrCode TpSemRelease(TpSemId sem)
{
    if (sem == NULL) {
        return TP_EINVAL;
    }
    if(osSemaphoreRelease((osSemaphoreId_t)sem) != osOK) {
        return TP_ERROR;
    }
    return TP_EOK;
}

void TpSemDelete(TpSemId sem)
{
    if (sem == NULL) {
        return;
    }
    osSemaphoreDelete((osSemaphoreId_t)sem);
}