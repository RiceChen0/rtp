#include "tp_def.h"
#include "cmsis_os2.h"

TpThreadId TpThreadCreate(tpThreadFunc func, void *argv, const TpThreadAttr *attr)
{
    osThreadId_t thread = NULL;
    osThreadAttr_t taskAttr = {
        .name = attr->name,
        .attr_bits = 0,
        .cb_mem = NULL,
        .cb_size = 0,
        .stack_mem = NULL,
        .stack_size = attr->stackSize,
        .priority = (osPriority_t)attr->priority,
        .tz_module = 0,
        .reserved = 0,
    };

    thread = osThreadNew((osThreadFunc_t)func, argv, &taskAttr);
    return (TpThreadId)thread;
}

void TpThreadDelete(TpThreadId thread)
{
    if(thread != NULL) {
        osThreadTerminate(thread);
    }
}
