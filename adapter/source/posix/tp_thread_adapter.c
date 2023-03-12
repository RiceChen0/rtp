#include "tp_def.h"
#include <pthread.h>

TpThreadId TpThreadCreate(tpThreadFunc func, void *argv, const TpThreadAttr *attr)
{
    (void)attr;
    pthread_t thread = 0;
    int errno = pthread_create(&thread, NULL, func, argv);
    if (errno != 0) {
        return NULL;
    }
    return (TpThreadId)thread;
}

void TpThreadDelete(TpThreadId thread)
{

}
