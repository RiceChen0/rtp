#include <stdio.h>

#include "rthread_pool.h"
#include "pthread.h"
#include "unistd.h"


static rtp pool;

void TestRtpHandle(void *argv)
{
    printf("%s--taskId: %d\r\n", __FUNCTION__, (uint32_t)argv);
}

int main(void)
{
    // ①
    rtp_create(&pool, "tp", 1024, 3);
    // ②
    rtp_add_task(&pool, TestRtpHandle, (void *)1);
    rtp_add_task(&pool, TestRtpHandle, (void *)2);
    rtp_add_task(&pool, TestRtpHandle, (void *)3);
    rtp_add_task(&pool, TestRtpHandle, (void *)4);
    rtp_add_task(&pool, TestRtpHandle, (void *)5);
    rtp_add_task(&pool, TestRtpHandle, (void *)6);

    return 0;
}
