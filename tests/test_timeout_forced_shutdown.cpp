#undef NDEBUG
#include <cassert>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "threadpool.h"

#define THREAD_CNT 6
#define TASK_CNT 8192
#define TIMEOUT 6
#define TASK_DURATION 5
pthread_mutex_t lock;
int done = 0;

void dummy_task(void* arg) {
    int* p_id = (int*)arg;
    *p_id += 1;
    sleep(TASK_DURATION);
    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char* argv[]) {
    pthread_mutex_init(&lock, NULL);
    threadpool_t* pool;
    pool = threadpool_create(THREAD_CNT, TASK_CNT - THREAD_CNT * 2);
    assert(pool != 0);

    int task_ids[TASK_CNT];
    printf("Adding %d jobs...\n", TASK_CNT);
    for (int i = 0; i < TASK_CNT; i++) {
        task_ids[i] = i;
        int ret = threadpool_add_task(pool, &dummy_task, (void*)(&task_ids[i]));
        assert(ret == 0);
    }
    // Approx. THREAD_CNT tasks will be finished until all tasks added to the queue
    printf("All jobs have been added\n");

    threadpool_timedwait_ret_t ret = threadpool_wait_for(pool, TIMEOUT);
    assert(ret == threadpool_timeout);
    
    if (ret == threadpool_timeout) {
        printf("Success: Timeout!\n");
    }

    assert(threadpool_destroy(pool) == 0);
    assert(threadpool_get_threads_working_cnt(pool) == 0);
    assert(threadpool_get_threads_alive_cnt(pool) == 0);
    printf("Finished tasks before timeout: %d\n", done);
    assert(done < TASK_CNT);
    assert(done > THREAD_CNT * (int)(TIMEOUT / TASK_DURATION) + THREAD_CNT);

   
    return 0;
}