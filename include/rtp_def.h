/*
 * Change Logs:
 * Date           Author       Notes
 * 2023-05-12     RiceChen     the first version
 */

#ifndef __RTP_DEF_H__
#define __RTP_DEF_H__

#ifdef __cplusplus
extern "C" {
#endif

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>

typedef enum {
    RTP_EOK = 0,                        /**< There is no error */
    RTP_ERROR,                          /**< A generic error happens */
    RTP_EFULL,                          /**< The resource is full */
    RTP_EEMPTY,                         /**< The resource is empty */
    RTP_ENOMEM,                         /**< No memory */
    RTP_EINVAL,                         /**< Invalid argument */
}rtp_err_t;

#define RTP_INLINE                      static __inline

/**
 * memory API
*/
#ifndef RTP_MALLOC
    #define RTP_MALLOC                  malloc
#endif

#ifndef RTP_FREE
    #define RTP_FREE                    free
#endif

#ifndef RTP_PRINT
    #define RTP_PRINT                   printf
#endif

#ifndef RTP_PRINT_TAG
    #define RTP_PRINT_TAG               "RTP"
#endif

#define RTP_LOGE(...)                   RTP_PRINT("\033[31;22m[E/%s](%s:%d) ", RTP_PRINT_TAG, __FUNCTION__, __LINE__);     \
                                        RTP_PRINT(__VA_ARGS__);                                                           \
                                        RTP_PRINT("\033[0m\n")
#define RTP_LOGI(...)                   RTP_PRINT("\033[32;22m[I/%s](%s:%d) ", RTP_PRINT_TAG, __FUNCTION__, __LINE__);     \
                                        RTP_PRINT(__VA_ARGS__);                                                           \
                                        RTP_PRINT("\033[0m\n")
#define RTP_LOGD(...)                   RTP_PRINT("[D/%s](%s:%d) ", RTP_PRINT_TAG, __FUNCTION__, __LINE__);                \
                                        RTP_PRINT(__VA_ARGS__);                                                           \
                                        RTP_PRINT("\n")

typedef void *rtp_task_id;
typedef void *rtp_mutex_id;
typedef void *rtp_sem_id;

/**
 * Task API
*/
struct rtp_task_attr{
    char *name;                 // name of the task
    uint32_t stack_size;        // size of stack
    uint8_t priority;           // initial task priority
};

typedef void(*rtp_task_func)(void *arg);

rtp_task_id rtp_task_create(rtp_task_func func, void *arg, const struct rtp_task_attr *attr);
void rtp_task_delete(rtp_task_id task);
rtp_err_t rtp_task_suspend(rtp_task_id task);
rtp_err_t rtp_task_resume(rtp_task_id task);

/**
 * Mutex API
*/
rtp_mutex_id rtp_mutex_create(void);
rtp_err_t rtp_mutex_lock(rtp_mutex_id mutex);
rtp_err_t rtp_mutex_unlock(rtp_mutex_id mutex);
void rtp_mutex_delete(rtp_mutex_id mutex);

/**
 * Sem API
*/
rtp_sem_id rtp_sem_create(uint32_t value);
rtp_err_t rtp_sem_lock(rtp_sem_id sem);
rtp_err_t rtp_sem_unlock(rtp_sem_id sem);
void rtp_sem_delete(rtp_sem_id sem);

struct rtp_list_node {
    struct rtp_list_node *next;
    struct rtp_list_node *prev;
};
typedef struct rtp_list_node rtp_list_t;

RTP_INLINE void rtp_list_init(rtp_list_t *l)
{
    l->next = l->prev = l;
}

RTP_INLINE void rtp_list_insert_after(rtp_list_t *l, rtp_list_t *n)
{
    l->next->prev = n;
    n->next = l->next;
    l->next = n;
    n->prev = l;
}

RTP_INLINE void rtp_list_insert_before(rtp_list_t *l, rtp_list_t *n)
{
    l->prev->next = n;
    n->prev = l->prev;
    l->prev = n;
    n->next = l;
}

RTP_INLINE void rtp_list_remove(rtp_list_t *n)
{
    n->next->prev = n->prev;
    n->prev->next = n->next;
    n->next = n->prev = n;
}

RTP_INLINE int rtp_list_is_empty(const rtp_list_t *l)
{
    return l->next == l;
}

RTP_INLINE int rtp_list_len(const rtp_list_t *l)
{
    int len = 0;
    const rtp_list_t *p = l;
    while (p->next != l) {
        p = p->next;
        len ++;
    }
    return len;
}

#define rtp_container_of(ptr, type, member)                                     \
    ((type *)((char *)(ptr) - (unsigned long)(&((type *)0)->member)))

#define rtp_list_obj_init(obj) {&(obj), &(obj)}

#define rtp_list_entry(node, type, member)                                      \
    rtp_container_of(node, type, member)

#define rtp_list_for_each(pos, head)                                            \
    for (pos = (head)->next; pos != (head); pos = pos->next)


#ifdef __cplusplus
}
#endif

#endif
