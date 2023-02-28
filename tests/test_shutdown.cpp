#undef NDEBUG
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include <cassert>

#include "threadpool.h"

#define THREADS 12
#define SIZE 5000
#define POOL_CNT 3
#define TASK_DURATION_US 180000
#define SHUTDOWN_DELAY 1

threadpool_t* pool[POOL_CNT];
long long int remaining;
pthread_mutex_t lock;

void dummy_task(void* arg) {
    int* p_id = (int*)arg;

    pthread_mutex_lock(&lock);
    *p_id = *p_id + 1;
    pthread_mutex_unlock(&lock);

    usleep(TASK_DURATION_US);

    pthread_mutex_lock(&lock);
    remaining--;
    pthread_mutex_unlock(&lock);
}

int main(int argc, char* argv[]) {
    remaining = SIZE * POOL_CNT;

    pthread_mutex_init(&lock, NULL);

    for (int i = 0; i < POOL_CNT; ++i) {
        pool[i] = threadpool_create(THREADS, SIZE);
        assert(pool[i] != 0);
        if (pool[i] == 0) {
            return -1;
        }
    }

    int task_ids[SIZE];

    for (int i = 0; i < SIZE; i++) {
        task_ids[i] = 0;
        for (int j = 0; j < POOL_CNT; ++j) {
            int ret = threadpool_add_task(pool[j], &dummy_task, (void*)(&task_ids[i]));
            assert(ret == 0);
        }
    }

    printf("All jobs have been added\n");
    printf("Shutting down in %d seconds...\n", SHUTDOWN_DELAY);

    sleep(SHUTDOWN_DELAY);

    for (int i = 0; i < POOL_CNT; ++i) {
        int working = threadpool_get_threads_working_cnt(pool[i]);
        //printf("Pool[%d] threads_working_cnt: %d\n", i, working);
        assert(working != 0);
    }

    for (int i = 0; i < POOL_CNT; ++i) {
        int ret1 = threadpool_destroy(pool[i]);
        printf("Pool[%d] Destroyed: %d\n", i, ret1);
        assert(ret1 == 0);
    }

    printf("Processed Tasks: %d\n", SIZE * POOL_CNT - remaining);

    assert(remaining > 0);

    return 0;
}