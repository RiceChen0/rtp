/*
 * Copyright (c) 2022 Hunan OpenValley Digital Industry Development Co., Ltd.
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 * 
 * Change Logs:
 * Date           Author       Notes
 * 2023-03-06     RiceChen     the first version
 */

#ifndef __TP_DEF_H__
#define __TP_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum{
    TP_EOK = 0,                         // There is no error
    TP_ERROR,                           // A generic error happens
    TP_ENOMEM,                          // No memory
    TP_EINVAL,                          // Invalid argument
} TpErrCode;

#define TP_PRINT                        printf

#define TP_LOGE(...)                    TP_PRINT("\033[31;22m[E/TP](%s:%d) ", __FUNCTION__, __LINE__);  \
                                        TP_PRINT(__VA_ARGS__);                                            \
                                        TP_PRINT("\033[0m\n")
#define TP_LOGI(...)                    TP_PRINT("\033[32;22m[I/TP](%s:%d) ", __FUNCTION__, __LINE__);  \
                                        TP_PRINT(__VA_ARGS__);                                            \
                                        TP_PRINT("\033[0m\n")
#define TP_LOGD(...)                    TP_PRINT("[D/TP](%s:%d) ", __FUNCTION__, __LINE__);             \
                                        TP_PRINT(__VA_ARGS__);                                            \
                                        TP_PRINT("\n")

typedef void *TpThreadId;
typedef void *TpMutexId;
typedef void *TpSemId;

/**
 * memory API
*/
#define TP_MALLOC                      malloc     
#define TP_FREE                        free

/**
 * Thread API
*/
typedef void *(*tpThreadFunc)(void *argv);

typedef struct {
    char *name;
    uint32_t stackSize;
    uint32_t priority : 8;
    uint32_t reserver : 24;
} TpThreadAttr;

TpThreadId TpThreadCreate(tpThreadFunc func, void *argv, const TpThreadAttr *attr);
void TpThreadDelete(TpThreadId thread);

/**
 * Mutex API
*/
TpMutexId TpMutexCreate(void);
TpErrCode TpMutexLock(TpMutexId mutex);
TpErrCode TpMutexUnlock(TpMutexId mutex);
void TpMutexDelete(TpMutexId mutex);

/**
 * Semaphore API
*/
TpSemId TpSemCreate(uint32_t value);
TpErrCode TpSemAcquire(TpSemId sem);
TpErrCode TpSemRelease(TpSemId sem);
void TpSemDelete(TpSemId sem);

#ifdef __cplusplus
}
#endif

#endif
