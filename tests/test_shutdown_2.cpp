#undef NDEBUG
#include <pthread.h>
#include <semaphore.h>
#include <stdio.h>
#include <unistd.h>

#include <cassert>

#include "threadpool.h"

#define THREADS 4
#define SIZE 5000
#define TASK_DURATION (5.0)
#define SHUTDOWN_DELAY_US (TASK_DURATION / 3 * 1E6)

long long int done;
int task_ids[SIZE];

threadpool_t* pool;
pthread_mutex_t lock;
sem_t sem;
void dummy_task(void* arg) {
    int* p_id = (int*)arg;

    pthread_mutex_lock(&lock);
    *p_id = *p_id + 1;
    pthread_mutex_unlock(&lock);

    sleep(TASK_DURATION);

    pthread_mutex_lock(&lock);
    done++;
    pthread_mutex_unlock(&lock);
}

void task_inserter(void* arg) {
    threadpool_t* pool = (threadpool_t*)arg;
    int inserted = 0;
    int rejected = 0;

    const int ATTEMPTS = 50;

    sem_wait(&sem);

    for (int i = 0; i < ATTEMPTS; ++i) {
        int ret = threadpool_add_task(pool, dummy_task, (void*)(&task_ids[0]));
        if (ret != 0) {
            rejected++;
        } else {
            inserted++;
        }
    }

    // printf("inserted: %d\n", inserted);
    // printf("rejected: %d\n", rejected);
    assert(inserted == THREADS - 1);
    assert(rejected == ATTEMPTS - THREADS + 1);
}

int main(int argc, char* argv[]) {
    done = 0;
    pthread_mutex_init(&lock, NULL);
    sem_init(&sem, 0, 0);
    pool = threadpool_create(THREADS, SIZE);

    assert(pool != 0);
    if (pool == 0) {
        return -1;
    }
    printf("Start...\n");

    int retval = threadpool_add_task(pool, task_inserter, (void*)(pool));  // task #1


    // Make the queue full
    for (int i = 0; i < SIZE - 1; i++) {
        task_ids[i] = 0;
        int ret = threadpool_add_task(pool, &dummy_task, (void*)(&task_ids[i]));
        assert(ret == 0);
    }

    sem_post(&sem);

    printf("All jobs have been added.\n");

    printf("Shutting down in %.1lf seconds...\n", SHUTDOWN_DELAY_US / 1E6);

    usleep(SHUTDOWN_DELAY_US);

    int working = threadpool_get_threads_working_cnt(pool);
    assert(working != 0);

    int ret1 = threadpool_destroy(pool);
    assert(ret1 == 0);

    int ret2 = threadpool_get_threads_alive_cnt(pool);
    assert(ret2 == 0);

    int ret3 = threadpool_get_threads_working_cnt(pool);
    assert(ret3 == 0);

    printf("Processed Dummy Tasks: %d\n", done);
    assert(done == THREADS - 1);
    assert(done > 0);

    return 0;
}