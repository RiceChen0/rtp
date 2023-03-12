#include "tp_def.h"
#include <semaphore.h>

TpSemId TpSemCreate(uint32_t value)
{
    sem_t *sem = (sem_t *)TP_MALLOC(sizeof(sem_t));
    if(sem == NULL) {
        return NULL;
    }
    sem_init(sem, 0, value);
    return (TpSemId)sem;
}

TpErrCode TpSemAcquire(TpSemId sem)
{
    if(sem == NULL) {
        return TP_EINVAL;
    }
    sem_wait((sem_t *)sem);
    return TP_EOK;
}

TpErrCode TpSemRelease(TpSemId sem)
{
    if(sem == NULL) {
        return TP_EINVAL;
    }
    sem_post((sem_t *)sem);
    return TP_EOK;
}

void TpSemDelete(TpSemId sem)
{
    if (sem == NULL) {
        return;
    }
    sem_destroy((sem_t *)sem);
    TP_FREE(sem);
    sem = NULL;
}