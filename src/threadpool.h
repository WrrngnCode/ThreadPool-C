
#ifndef _THREADPOOL_H_
#define _THREADPOOL_H_

#ifdef __cplusplus
extern "C" {
#endif
#include <pthread.h>
#include <errno.h>
typedef struct threadpool threadpool_t;
typedef enum threadpool_timedwait_ret{ 
    threadpool_error = -1,
    threadpool_finished = 0,
    threadpool_timeout = ETIMEDOUT,
} threadpool_timedwait_ret_t;

threadpool_t* threadpool_create(int thread_count, int taskqueue_size);
int threadpool_add_task(threadpool_t* pool, void (*function)(void*), void* argument);
void threadpool_wait(threadpool_t* pool);
int threadpool_destroy(threadpool_t* pool);
int threadpool_get_task_cnt(threadpool_t* pool);
int threadpool_get_threads_working_cnt(threadpool_t* pool);
int threadpool_get_threads_alive_cnt(threadpool_t* pool);
threadpool_timedwait_ret_t threadpool_wait_for(threadpool_t* pool, int timeout_sec);
#ifdef __cplusplus
}
#endif

#endif /* _THREADPOOL_H_ */