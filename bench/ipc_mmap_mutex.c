/*
 * IPC mutex overhead
 *
 * output format: list of time in nanoseconds, seperated by newline
 *
 */

#include "ipc_bench.h"

struct shared_mem {
    pthread_mutex_t mutex;

    struct timespec start;
    bool sent, ack;
};

void send(struct shared_mem *shmp) {
    pthread_mutex_lock(&shmp->mutex);
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
    
    struct timespec end, diff;
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (record) {
        diff.tv_sec = end.tv_sec - shmp->start.tv_sec;
        diff.tv_nsec = end.tv_nsec - shmp->start.tv_nsec;
        assert(diff.tv_nsec >= 0);

        printf("%ld\n",diff.tv_nsec + (diff.tv_sec * 1000000000));
    }

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
    check(pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED), "failed to set mutex shared attr");
    check(pthread_mutex_init(&shmp->mutex, &mutex_attr), "failed to init mutex");

    shmp->sent  = false;
    shmp->ack   = false;

    switch(fork()) {
    case -1: // fail
        fprintf(stderr, "failed to fork()\n");
        exit(-1);

    case 0: // child
        for (int i = 0; i < NUM_WARMUP; i++)
            recv(shmp, false);

        for (int i = 0; i < NUM_ITER; i++)
            recv(shmp, true);
        break;

    default: // parent
        for (int i = 0; i < NUM_WARMUP; i++)
            send(shmp);

        for (int i = 0; i < NUM_ITER; i++)
            send(shmp);

        wait(NULL);
    }
}
