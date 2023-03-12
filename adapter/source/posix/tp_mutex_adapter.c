#include "tp_def.h"
#include <pthread.h>

TpMutexId TpMutexCreate(void)
{
    pthread_mutex_t *mutex = TP_MALLOC(sizeof(pthread_mutex_t));
    if (mutex == NULL) {
        return NULL;
    }
    (void)pthread_mutex_init(mutex, NULL);
    return (TpMutexId)mutex;
}

TpErrCode TpMutexLock(TpMutexId mutex)
{
    if (mutex == NULL) {
        return TP_ERROR;
    }
    pthread_mutex_lock((pthread_mutex_t *)mutex);
    return TP_EOK;
}

TpErrCode TpMutexUnlock(TpMutexId mutex)
{
    if (mutex == NULL) {
        return TP_ERROR;
    }
    pthread_mutex_unlock((pthread_mutex_t *)mutex);
    return TP_EOK;
}

void TpMutexDelete(TpMutexId mutex)
{
    if (mutex == NULL) {
        return;
    }
    pthread_mutex_destroy(mutex);
    TP_FREE(mutex);
    mutex = NULL;
}
