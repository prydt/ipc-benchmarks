/*
 * IPC mutex overhead
 *
 * output format: list of time in seconds, seperated by newline
 *
 */

#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>
#include <sched.h>

#define NUM_ITER (1000 * 10)
#define NUM_WARMUP 1000

void check(int ret, const char* errormsg) {
    if (ret != 0) {
        printf("error %s\n", errormsg);
        exit(-1);
    }
}

struct shared_mem {
    // pthread_cond_t cv_sent, cv_ack;
    pthread_mutex_t mutex;
    // sem_t init_sem; // used for initialization

    struct timespec start;
    bool sent, ack;//, done;
};

void send(struct shared_mem *shmp) {
    pthread_mutex_lock(&shmp->mutex);
    // shmp->start = clock();
    clock_gettime(CLOCK_MONOTONIC, &shmp->start);
    shmp->ack = false;
    shmp->sent = true;
    msync(shmp, sizeof(struct shared_mem), MS_ASYNC | MS_INVALIDATE);
    pthread_mutex_unlock(&shmp->mutex);

    while (!shmp->ack)
        sched_yield();
}

void recv(struct shared_mem *shmp, bool record) {
    while(!shmp->sent)
        sched_yield();

    pthread_mutex_lock(&shmp->mutex);
    
    // clock_t end = clock();
    struct timespec end, diff;
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (record) {
        diff.tv_sec = end.tv_sec - shmp->start.tv_sec;
        diff.tv_nsec = end.tv_nsec - shmp->start.tv_nsec;
        assert(diff.tv_nsec >= 0);

        printf("%ld\n",diff.tv_nsec + (diff.tv_sec * 1000000000));
        // printf("%Lf\n", (long double) (end - shmp->start) / CLOCKS_PER_SEC);
        // printf("start: %ld, end: %ld, end - start = %ld\n", shmp->start, end, end - shmp->start);
    }
    // shmp->start=0;

    shmp->sent = false;
    shmp->ack = true;
    msync(shmp, sizeof(struct shared_mem), MS_ASYNC | MS_INVALIDATE);
    pthread_mutex_unlock(&shmp->mutex);
}

int main() {

    struct shared_mem *shmp = mmap( NULL, sizeof(struct shared_mem),
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS,
                                    -1,
                                    0);

    if (shmp == MAP_FAILED)
        return -1;

    pthread_mutexattr_t mutex_attr;
    check(pthread_mutexattr_init(&mutex_attr), "failed to init mutex attr struct");
    check(pthread_mutexattr_settype(&mutex_attr, PTHREAD_MUTEX_ERRORCHECK), "failed to set mutex attr type");
    check(pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED), "failed to set mutex shared attr");
    check(pthread_mutex_init(&shmp->mutex, &mutex_attr), "failed to init mutex");

    // pthread_condattr_t cond_attr;
    // check(pthread_condattr_init(&cond_attr), "failed to init condvar attr struct");
    // check(pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED), "failed to set condvar shared attr");
    // check(pthread_cond_init(&shmp->cv_sent, &cond_attr), "failed to init cv_sent");
    // check(pthread_cond_init(&shmp->cv_ack, &cond_attr), "failed to init cv_ack");

    // sem_init(&shmp->init_sem, /* non-zero value = shared between processes */ 1, 0);

    shmp->sent  = false;
    shmp->ack   = false;
    // shmp->done  = false;

    pid_t child = fork();
    if (child == 0) {
        // in child process (Receiver)

        // sem_post(&shmp->init_sem);

        for (int i = 0; i < NUM_WARMUP; i++)
            recv(shmp, false);

        for (int i = 0; i < NUM_ITER; i++)
            recv(shmp, true);


        // pthread_mutex_lock(&shmp->mutex);
        // shmp->done = true;
        // pthread_cond_signal(&shmp->cv_ack);
        // pthread_mutex_unlock(&shmp->mutex);
    } else {
        // parent process (Sender)

        // sem_wait(&shmp->init_sem); // always start sender after receiver

        for (int i = 0; i < NUM_WARMUP; i++)
            send(shmp);

        for (int i = 0; i < NUM_ITER; i++)
            send(shmp);

        // wait for child process to finish
        wait(NULL);
    }
}
