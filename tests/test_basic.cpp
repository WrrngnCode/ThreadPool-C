#undef NDEBUG
#include <cassert>
#include <pthread.h>
#include <stdio.h>
#include <unistd.h>

#include "threadpool.h"

#define THREADS 5
#define SIZE 1500


void dummy_task(void* arg) {
    int* p_id = (int*)arg;
    *p_id += 1;
    usleep(8000);
}

int main(int argc, char* argv[]) {
    threadpool_t* pool;
    pool = threadpool_create(THREADS, 10);
    assert(pool != 0);
    if (pool == 0) {
        return -1;
    }

    int task_ids[SIZE];
    printf("Adding %d jobs...\n", SIZE);
    for (int i = 0; i < SIZE; i++) {
        task_ids[i] = i;
        int ret = threadpool_add_task(pool, &dummy_task, (void*)(&task_ids[i]));
        assert(ret == 0);
    }
    printf("All jobs have been added.\n");
    printf("Waiting for jobs to finish...\n");
    threadpool_wait(pool);

    for (int i = 0; i < SIZE; i++) {
        int copy = task_ids[i];
        assert(task_ids[i] == i + 1);
    }

    printf("All jobs have been finished.\n");
    int ret = threadpool_destroy(pool);
    assert(ret == 0);

    return 0;
}