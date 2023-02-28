
#include "threadpool.h"

#include <assert.h>
#include <pthread.h>
#include <pthread_time.h>
#include <semaphore.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>

#define MAX_THREADS 12
#define MAX_TASKQUEUE 8192

// https://github.com/Pithikos/C-Thread-Pool/blob/master/thpool.c
// https://github.com/mbrossard/threadpool/blob/master/src/threadpool.c
// https://github.com/ericomeehan/libeom/blob/main/Systems/ThreadPool.c


#ifdef NDEBUG
#undef THREADPOOL_DEBUG
#endif

#ifdef THREADPOOL_DEBUG
#define THREADPOOL_DEBUG 1
#else
#define THREADPOOL_DEBUG 0
#endif

#if !defined(DISABLE_PRINT) && THREADPOOL_DEBUG == 1
#define log_err(str) fprintf(stderr, str)
#define log_info(str) fprintf(stdout, str)
#else
#define log_err(str)
#define log_info(str)
#endif

pthread_mutex_t mtx = PTHREAD_MUTEX_INITIALIZER;  // mutex for the queue
pthread_cond_t cond;
#define MAX_MSG 300
static char message[MAX_MSG];

typedef struct {
    void (*taskFunction)(void*);
    void* argument;
} task_t;

typedef struct threadpool {
    pthread_mutex_t lock;
    pthread_mutex_t queue_lock;
    pthread_cond_t notify;            // Condition variable to notify worker threads.
    pthread_cond_t threads_all_done;  // signal to stop waiting
    pthread_t* threads;
    task_t* task_queue;
    int task_cnt;        // number of remaining tasks in the queue (pending tasks)
    int taskqueue_size;  // Size of the task queue.
    int front;           // Index of the next element.
    int rear;            // Index of one past the last item.
    int thread_count;    // number of threads in the pool.
    int threads_alive_cnt;
    int threads_working_cnt;
    int shutdown;
} threadpool_t;

static int threadpool_free(threadpool_t* pool);
int threadpool_destroy(threadpool_t* pool);
static void* threadpool_worker(void* threadpool);

threadpool_t* threadpool_create(int thread_count, int taskqueue_size) {
    if (thread_count <= 0 ||
        thread_count > MAX_THREADS ||
        taskqueue_size <= 0 || taskqueue_size > MAX_TASKQUEUE) {
        log_err("Invalid parameters to initialize the pool.");
        return NULL;
    }

    threadpool_t* pool = (threadpool_t*)malloc(sizeof(threadpool_t));
    if (pool == NULL) {
        return NULL;
    }

    pool->threads_alive_cnt = 0;
    pool->threads_working_cnt = 0;
    pool->taskqueue_size = taskqueue_size;
    pool->task_cnt = 0;
    pool->front = 0;
    pool->rear = 0;
    pool->shutdown = 0;
    pool->thread_count = 0;

    pool->threads = malloc(sizeof(pthread_t) * (size_t)thread_count);
    pool->task_queue = malloc(sizeof(task_t) * (size_t)taskqueue_size);

    const int res1 = pthread_mutex_init(&pool->lock, NULL);
    const int res2 = pthread_mutex_init(&pool->queue_lock, NULL);
    const int res3 = pthread_cond_init(&pool->notify, NULL);
    const int res4 = pthread_cond_init(&pool->threads_all_done, NULL);
    if (res1 != 0 || res2 != 0 || res3 != 0 || res4 != 0) {
        perror("Failed to initialize mutex or condition variable\n");
        return NULL;
    }

    // Start worker threads
    for (int i = 0; i < thread_count; i++) {
        if (pthread_create(&(pool->threads[i]), NULL, threadpool_worker, (void*)pool) != 0) {
            perror("Failed to start worker thread. Destroying thread pool now...\n");
            threadpool_destroy(pool);
            return NULL;
        }
        pool->thread_count++;
    }

    while (pool->threads_alive_cnt != thread_count) {
        usleep(200);
    }

    return pool;
}

/* add task to the queue. Function is blocking if queue is full */
int threadpool_add_task(threadpool_t* pool, void (*function)(void*), void* argument) {
    int err = 0;

    if (pool == NULL || function == NULL) {
        return -1;
    }

    // Lock the queue
    if (pthread_mutex_lock(&pool->queue_lock) != 0) {
        return -1;
    }

    // wait if queue is full
    while ((pool->rear + 1) % pool->taskqueue_size == pool->front && !pool->shutdown) {
        pthread_cond_wait(&pool->notify, &pool->queue_lock);
    }

    if (!pool->shutdown) {
        task_t current_task;
        current_task.taskFunction = function;
        current_task.argument = argument;

        // insert new task
        pool->task_queue[pool->rear] = current_task;
        pool->rear = (pool->rear + 1) % pool->taskqueue_size;
        pool->task_cnt++;  // increment the number of pending tasks.
        
        // Notify a worker thread that there is task in the queue
        pthread_cond_signal(&pool->notify);

    } else {
        err = -1;  // shutting down
    }

    pthread_mutex_unlock(&pool->queue_lock);
    return err;
}

// force worker thread shutdown and destroy pool
int threadpool_destroy(threadpool_t* pool) {
    int err = 0;

    if (pool == NULL) {
        return -1;
    }

    if (pthread_mutex_lock(&pool->queue_lock) != 0) {
        return -1;
    }

    if (pool->shutdown) {
        return -1;
    }

    log_info("Shutting down...\n");
    pool->shutdown = 1;

    // wake up worker threads so that they can check shutdown condition
    if (pthread_cond_broadcast(&pool->notify) != 0) {
        perror("Signal error.\n");
    }

    if (pthread_mutex_unlock(&pool->queue_lock) != 0) {
        perror("Mutex unlock error.\n");
        return -1;
    }

    // Poll remaining threads
    while (pool->threads_alive_cnt != 0) {
        pthread_cond_broadcast(&pool->notify);
        sleep(1);
    }

    // Join worker threads
    for (int i = 0; i < pool->thread_count; i++) {
        if (pthread_join(pool->threads[i], NULL) != 0) {
            err = -1;
            perror("Error joining pthread.\n");
        }
    }

    // Deallocate pool
    if (!err) {
        threadpool_free(pool);
    }

    return err;
}


static int threadpool_free(threadpool_t* pool) {
    if (pool == NULL || pool->threads_alive_cnt > 0) {
        return -1;
    }
    if (pool->threads) {
        free(pool->threads);
        free(pool->task_queue);
        pthread_mutex_lock(&(pool->queue_lock));
        pthread_mutex_destroy(&(pool->queue_lock));
        pthread_mutex_destroy(&(pool->lock));
        pthread_cond_destroy(&(pool->notify));
        pthread_cond_destroy(&(pool->threads_all_done));
    }
    free(pool);
    return 0;
}


static void* threadpool_worker(void* threadpool) {
    threadpool_t* pool = (threadpool_t*)threadpool;
    task_t task;

    pthread_mutex_lock(&pool->lock);
    pool->threads_alive_cnt++;
    pthread_mutex_unlock(&pool->lock);

    while (!pool->shutdown) {
        // wait for tasks if task queue is empty
        pthread_mutex_lock(&pool->queue_lock);
        while (pool->rear == pool->front && !pool->shutdown) {
            pthread_cond_wait(&pool->notify, &pool->queue_lock);
        }

        if (!pool->shutdown) {
            pthread_mutex_lock(&pool->lock);
            pool->threads_working_cnt++;
            pthread_mutex_unlock(&pool->lock);

            // copy the task function pointer from the queue and execute
            task.taskFunction = pool->task_queue[pool->front].taskFunction;
            task.argument = pool->task_queue[pool->front].argument;
            pool->task_cnt--;
            pool->front = (pool->front + 1) % pool->taskqueue_size;

            pthread_cond_signal(&pool->notify);

            pthread_mutex_unlock(&pool->queue_lock);

            task.taskFunction(task.argument);

            pthread_mutex_lock(&pool->lock);
            pool->threads_working_cnt--;
            if (pool->threads_working_cnt == 0 && pool->task_cnt == 0) {
                pthread_cond_signal(&pool->threads_all_done);
            }
            pthread_mutex_unlock(&pool->lock);
        } else {
            log_info("Shutdown flag was set.\n");
        }
    }

    pthread_mutex_unlock(&pool->queue_lock);
    pthread_mutex_lock(&pool->lock);
    pool->threads_alive_cnt--;
    pthread_mutex_unlock(&pool->lock);
    pthread_exit(NULL);
    return NULL;
}

/* Wait until all jobs have been finished */
void threadpool_wait(threadpool_t* pool) {
    pthread_mutex_lock(&pool->lock);

    // wait until task queue is empty and all worker threads are finished
    while (pool->threads_working_cnt || pool->task_cnt) {
        pthread_cond_wait(&pool->threads_all_done, &pool->lock);
    }
    pthread_mutex_unlock(&pool->lock);
    log_info("Finished waiting. No more work to be done.\n");
}

/* wait until all jobs have been finished or timeout has passed*/
threadpool_timedwait_ret_t threadpool_wait_for(threadpool_t* pool, int timeout_sec) {
    threadpool_timedwait_ret_t retval = threadpool_finished;
    struct timespec tp_wait_until = {0, 0};  // time point wait until
    const int ret_gettime = clock_gettime(CLOCK_REALTIME, &tp_wait_until);

    if (ret_gettime) {
        log_err("Couldn't get time from the clock.\n");
    }

    tp_wait_until.tv_sec += timeout_sec;

    pthread_mutex_lock(&pool->lock);
    const int ret = pthread_cond_timedwait(&pool->threads_all_done,
                                           &pool->lock,
                                           &tp_wait_until);
    pthread_mutex_unlock(&pool->lock);

    switch (ret) {
        case 0:
            snprintf(message, MAX_MSG, "%d tasks pending. Threads Working: %d\n", pool->task_cnt, pool->threads_working_cnt);
            log_info(message);
            break;
        case ETIMEDOUT:
            log_info("The timeout has passed.\n");
            retval = threadpool_timeout;
            break;
        case EINVAL:
            log_err("The value specified by abstime, cond or mutex is invalid.\n");
            retval = threadpool_error;
            break;
        case EPERM:
            log_err("The mutex was not owned by the current thread at the time of the call.\n");
            retval = threadpool_error;
            break;
        default:
            break;
    }
    return retval;
}

int threadpool_get_threads_working_cnt(threadpool_t* pool) {
    pthread_mutex_lock(&pool->lock);
    int copy = pool->threads_working_cnt;
    pthread_mutex_unlock(&pool->lock);
    return copy;
}

int threadpool_get_threads_alive_cnt(threadpool_t* pool) {
        pthread_mutex_lock(&pool->lock);
        int copy = pool->threads_alive_cnt;
        pthread_mutex_unlock(&pool->lock);
        return copy;
}

int threadpool_get_task_cnt(threadpool_t* pool) {
    pthread_mutex_lock(&pool->queue_lock);
    int copy = pool->task_cnt;
    pthread_mutex_unlock(&pool->queue_lock);
    return copy;
}