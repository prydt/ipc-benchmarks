/*
 * IPC semaphore overhead
 *
 * output format: list of time in seconds, seperated by newline
 *
 */

#include "ipc_bench.h"

struct shared_mem {
    sem_t lock;

    struct timespec start;
    bool sent, ack;
};

void send(struct shared_mem *shmp) {
    sem_wait(&shmp->lock);
    clock_gettime(CLOCK_MONOTONIC, &shmp->start);
    shmp->ack = false;
    shmp->sent = true;
    msync(shmp, sizeof(struct shared_mem), MS_ASYNC | MS_INVALIDATE);
    sem_post(&shmp->lock);

    while (!shmp->ack)
        sched_yield();
}

void recv(struct shared_mem *shmp, bool record) {
    while(!shmp->sent)
        sched_yield();

    sem_wait(&shmp->lock);
    
    struct timespec end, diff;
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (record) {
        diff.tv_sec = end.tv_sec - shmp->start.tv_sec;
        diff.tv_nsec = end.tv_nsec - shmp->start.tv_nsec;
        // assert(diff.tv_nsec >= 0);
        if (diff.tv_nsec < 0) {
            diff.tv_sec--;
            diff.tv_nsec += 1000000000;
        }

        printf("%ld\n",diff.tv_nsec + (diff.tv_sec * 1000000000));
    }

    shmp->sent = false;
    shmp->ack = true;
    msync(shmp, sizeof(struct shared_mem), MS_ASYNC | MS_INVALIDATE);
    sem_post(&shmp->lock);
}

int main() {

    struct shared_mem *shmp = mmap( NULL, sizeof(struct shared_mem),
                                    PROT_READ | PROT_WRITE,
                                    MAP_SHARED | MAP_ANONYMOUS,
                                    -1,
                                    0);

    if (shmp == MAP_FAILED)
        return -1;

    check(sem_init(&shmp->lock, /* non-zero value = shared between processes */ 1, 1), "failed to initialize semaphore\n");

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
