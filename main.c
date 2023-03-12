#include <stdio.h>
#include "tp_manage.h"
#include "pthread.h"
#include "unistd.h"

void TestTaskHandle(void *argv)
{
    printf("%s\r\n", __FUNCTION__);
}

int main(int argc, char *argv[])
{
    Tp pool;

    TpCreate(&pool, "tp", 1024, 3);

    TpAddTask(&pool, TestTaskHandle, NULL);
    
    while(1) {
        sleep(1);
    }
    return 0;
}
