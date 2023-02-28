#undef NDEBUG
#include <cassert>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "threadpool.h"

#define THREAD 8
#define SIZE 8192
#define POOL_CNT 16

threadpool_t* pool[POOL_CNT];
int remaining;
pthread_mutex_t lock;

void dummy_task(void* arg) {
    int* p_id = (int*)arg;
    *p_id += 1;

    if (*p_id < POOL_CNT) {
        int ret = threadpool_add_task(pool[*p_id], &dummy_task, arg);
        assert(ret == 0);
    } else {
        pthread_mutex_lock(&lock);
        remaining--;
        pthread_mutex_unlock(&lock);
    }
}

int main(int argc, char* argv[]) {
    remaining = SIZE;
    int copy = 1;

    pthread_mutex_init(&lock, NULL);


    for (int i = 0; i < POOL_CNT; ++i) {
        pool[i] = threadpool_create(THREAD, SIZE);
        assert(pool[i] != 0);
        if (pool[i] == 0) {
            return -1;
        }
    }

    sleep(1);

    int task_ids[SIZE];
    for (int i = 0; i < SIZE; i++) {
        task_ids[i] = 0;
        int ret = threadpool_add_task(pool[0], &dummy_task, (void*)(&task_ids[i]));
        assert(ret == 0);
    }
    printf("All jobs have been added\n");

    while (copy > 0) {
        usleep(5000);
        pthread_mutex_lock(&lock);
        copy = remaining;
        pthread_mutex_unlock(&lock);
    }

    for (int i = 0; i < POOL_CNT; ++i) {
        int working = threadpool_get_threads_working_cnt(pool[i]);
        assert(working == 0);
        int ret = threadpool_destroy(pool[i]);
        assert(ret == 0);
    }

    for (int i = 0; i < SIZE; i++) {
        assert(task_ids[i] == POOL_CNT);
    }

    //pthread_mutex_destroy(&lock);
    printf("Done.");
    return 0;
}