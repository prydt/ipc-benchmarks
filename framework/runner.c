#include <assert.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/mman.h>
#include <sys/wait.h>
#include <semaphore.h>
#include <pthread.h>
#include <unistd.h>
#include <errno.h>

#define ERROR(message) do { \
        fprintf(stderr, "ERROR (%s:%d) %s\n", __FILE__, __LINE__, message); \
        exit(-1); \
    } while(0)

#define NUM_ITER (1000 * 10)
#define NUM_WARMUP 1000

void check(int ret, const char* errormsg) {
    if (ret != 0) {
        fprintf(stderr, "error %s, ERR: %s\n", errormsg, strerror(errno));
        exit(-1);
    }
}

enum data_mechanism {
    MMAP,

    DATA_NONE
};

enum sync_mechanism {
    CONDITION_VARIABLES,
    MUTEX,
    SEMAPHORE,
    ATOMICS,
    SPINLOCKS,
    SIGNALS,

    SYNC_NONE
};

enum combined_mechanism {
    PIPES,
    SOCKETS,
    MESSAGE_QUEUES,

    COMBINED_NONE
};

struct benchmark_config {
    enum data_mechanism     data;
    enum sync_mechanism     sync;
    enum combined_mechanism combined;
};

struct condvar_data {
    // pthread_cond_t cv_sent, cv_ack;
    pthread_cond_t cv_empty, cv_full;
    pthread_mutex_t mutex;

    int payload;

    bool closed, empty;

    // bool sent, acked, done;
};

void *shared_memory;

void ipc_init(struct benchmark_config *config) {
    if (config->data == MMAP && config->sync == CONDITION_VARIABLES) {
        shared_memory = mmap(NULL, sizeof(struct condvar_data),
            PROT_READ | PROT_WRITE,
            MAP_SHARED | MAP_ANONYMOUS, -1, 0);
        
        if (shared_memory == MAP_FAILED)
            ERROR("mmap failed");

        struct condvar_data *ptr = shared_memory;

        pthread_mutexattr_t mutex_attr;
        check(pthread_mutexattr_init(&mutex_attr), "failed to init mutex attr struct");
        check(pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED), "failed to set mutex shared attr");
        check(pthread_mutex_init(&ptr->mutex, &mutex_attr), "failed to init mutex");

        pthread_condattr_t cond_attr;
        check(pthread_condattr_init(&cond_attr), "failed to init condvar attr struct");
        check(pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED), "failed to set condvar shared attr");
        // check(pthread_cond_init(&ptr->cv_sent, &cond_attr), "failed to init cv_sent");
        // check(pthread_cond_init(&ptr->cv_ack, &cond_attr), "failed to init cv_ack");

        check(pthread_cond_init(&ptr->cv_empty, &cond_attr), "failed to init cv_sent");
        check(pthread_cond_init(&ptr->cv_full, &cond_attr), "failed to init cv_ack");

        // ptr->sent = false;
        // ptr->acked = false;
        // ptr->done = false;
        ptr->closed = false;
        ptr->empty = true;
    }
}

void condvar_send(int round) {
    struct condvar_data *shmp = shared_memory;
    pthread_mutex_lock(&shmp->mutex);

    if (shmp->closed) {
        ERROR("tried to send in closed buffer");
    }

    while (!shmp->empty)
        pthread_cond_wait(&shmp->cv_full, &shmp->mutex);
    
    // there is now an empty slot
    shmp->payload = round;
    shmp->empty = false;
    pthread_cond_signal(&shmp->cv_empty);

    // shmp->payload = round;

    // shmp->acked = false;
    // shmp->sent = true;
    // // msync(shmp, sizeof(struct condvar_data), MS_SYNC | MS_INVALIDATE);

    // pthread_cond_signal(&shmp->cv_sent);

    // while (!shmp->acked && !shmp->done)
    //     pthread_cond_wait(&shmp->cv_ack, &shmp->mutex);

    // shmp->sent = false;
    // shmp->acked = false;

    pthread_mutex_unlock(&shmp->mutex);
}

void condvar_recv(int expected_round) {
    struct condvar_data *shmp = shared_memory;
    pthread_mutex_lock(&shmp->mutex);

    while (shmp->empty) {
        if (shmp->closed)
            ERROR("tried to get in closed buffer");
        
        pthread_cond_wait(&shmp->cv_empty, &shmp->mutex);
    }

    // printf("%d =? %d\n",shmp->payload, expected_round);
    assert(shmp->payload == expected_round); // sanity check
    shmp->empty = true;

    pthread_cond_signal(&shmp->cv_full);


    // while(!shmp->sent && !shmp->done)
    //     pthread_cond_wait(&shmp->cv_sent, &shmp->mutex);

    // // msync(shmp, sizeof(struct condvar_data), MS_SYNC | MS_INVALIDATE);
    // printf("%d =? %d\n",shmp->payload, expected_round);
    // assert(shmp->payload == expected_round); // sanity check
    
    // shmp->acked = true;
    // pthread_cond_signal(&shmp->cv_ack);
    pthread_mutex_unlock(&shmp->mutex);
}

void ipc_send(struct benchmark_config *config, int round) {
    if (config->data == MMAP && config->sync == CONDITION_VARIABLES)
        condvar_send(round);
}

void ipc_recv(struct benchmark_config *config, int expected_round) {
    if (config->data == MMAP && config->sync == CONDITION_VARIABLES)
        condvar_recv(expected_round);
}

void child_warmup(struct benchmark_config *config) {
    for (int i = 0; i < NUM_WARMUP; i++) {
        ipc_recv(config, i);
        // ipc_send(config, i);
    }

    //TODO make general clean up function?
    // ((struct condvar_data*) shared_memory)->done = true;
}
void parent_warmup(struct benchmark_config *config) {
    for (int i = 0; i < NUM_WARMUP; i++) {
        ipc_send(config, i);
        // ipc_recv(config, i);
    }

    // TODO
    // ((struct condvar_data*) shared_memory)->done = true;
}

void child_benchmark(struct benchmark_config *config) {
    // ((struct condvar_data*) shared_memory)->done = false;
    for (int i = 0; i < NUM_ITER; i++) {
        ipc_recv(config, i);
        // ipc_send(config, i);
    }

    // TODO
    // pthread_mutex_lock()
    // struct condvar_data *ptr = shared_memory;
    // pthread_mutex_lock(&ptr->mutex);
    // ptr->done = true;
    // pthread_mutex_unlock(&ptr->mutex);
}
void parent_benchmark(struct benchmark_config *config) {
    // ((struct condvar_data*) shared_memory)->done = false;
    for (int i = 0; i < NUM_ITER; i++) {
        ipc_send(config, i);
        // ipc_recv(config, i);
    }

    // TODO
    // ((struct condvar_data*) shared_memory)->done = true;
}

int main(int argc, char **argv) {

    // TODO read arguments
    // enum data_mechanism data = MMAP;
    // enum sync_mechanism sync = CONDITION_VARIABLES;
    // enum combined_mechanism combined = COMBINED_NONE;

    struct benchmark_config cfg = {MMAP, CONDITION_VARIABLES, COMBINED_NONE};

    // TODO set CPU affinity

    const long PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    ipc_init(&cfg);

    switch(fork()) {
    case -1: // FAIL
        ERROR("failed to fork()");

    case 0: // child

        // sem_post(&init_sem);

        child_warmup(&cfg);
        child_benchmark(&cfg);

        break;
    default: // parent 

        // sem_wait(&init_sem);

        parent_warmup(&cfg);

        struct timespec start, end, diff;
        clock_gettime(CLOCK_MONOTONIC, &start);
        parent_benchmark(&cfg);
        clock_gettime(CLOCK_MONOTONIC, &end);
        diff.tv_sec = end.tv_sec - start.tv_sec;
        diff.tv_nsec = end.tv_nsec - start.tv_nsec;
        printf("time: %ld\n", (diff.tv_sec * 1000000000) + diff.tv_nsec);

        wait(NULL);
    }


    return EXIT_SUCCESS;
}