#include "ipc_runner.h"

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
        shared_memory =
            mmap(NULL, sizeof(struct condvar_data), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        if (shared_memory == MAP_FAILED) ERROR("mmap failed");

        struct condvar_data *ptr = shared_memory;

        pthread_mutexattr_t mutex_attr;
        check(pthread_mutexattr_init(&mutex_attr),
              "failed to init mutex attr struct");
        check(pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED),
              "failed to set mutex shared attr");
        check(pthread_mutex_init(&ptr->mutex, &mutex_attr),
              "failed to init mutex");

        pthread_condattr_t cond_attr;
        check(pthread_condattr_init(&cond_attr),
              "failed to init condvar attr struct");
        check(pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED),
              "failed to set condvar shared attr");
        // check(pthread_cond_init(&ptr->cv_sent, &cond_attr), "failed to init
        // cv_sent"); check(pthread_cond_init(&ptr->cv_ack, &cond_attr), "failed
        // to init cv_ack");

        check(pthread_cond_init(&ptr->cv_empty, &cond_attr),
              "failed to init cv_sent");
        check(pthread_cond_init(&ptr->cv_full, &cond_attr),
              "failed to init cv_ack");

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

    while (!shmp->empty) pthread_cond_wait(&shmp->cv_full, &shmp->mutex);

    // there is now an empty slot
    shmp->payload = round;
    shmp->empty = false;
    pthread_cond_signal(&shmp->cv_empty);

    pthread_mutex_unlock(&shmp->mutex);
}

void condvar_recv(int expected_round) {
    struct condvar_data *shmp = shared_memory;
    pthread_mutex_lock(&shmp->mutex);

    while (shmp->empty) {
        if (shmp->closed) ERROR("tried to get in closed buffer");

        pthread_cond_wait(&shmp->cv_empty, &shmp->mutex);
    }

    assert(shmp->payload == expected_round);  // sanity check
    shmp->empty = true;

    pthread_cond_signal(&shmp->cv_full);

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
    }
}
void parent_warmup(struct benchmark_config *config) {
    for (int i = 0; i < NUM_WARMUP; i++) {
        ipc_send(config, i);
    }
}

void child_benchmark(struct benchmark_config *config) {
    // ((struct condvar_data*) shared_memory)->done = false;
    for (int i = 0; i < NUM_ITER; i++) {
        ipc_recv(config, i);
        // ipc_send(config, i);
    }
}
void parent_benchmark(struct benchmark_config *config) {
    // ((struct condvar_data*) shared_memory)->done = false;
    for (int i = 0; i < NUM_ITER; i++) {
        ipc_send(config, i);
    }
}

int main(int argc, char **argv) {
    // TODO read arguments
    struct benchmark_config cfg = {MMAP, CONDITION_VARIABLES, COMBINED_NONE};

    // TODO set CPU affinity
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    check(pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset),
          "failed to set CPU affinity");

    const long PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    ipc_init(&cfg);

    switch (fork()) {
        case -1:  // FAIL
            ERROR("failed to fork()");

        case 0:  // child

            // sem_post(&init_sem);

            child_warmup(&cfg);
            child_benchmark(&cfg);

            break;
        default:  // parent

            // sem_wait(&init_sem);

            parent_warmup(&cfg);

            struct timespec start, end, diff;
            clock_gettime(CLOCK_MONOTONIC, &start);
            parent_benchmark(&cfg);
            clock_gettime(CLOCK_MONOTONIC, &end);
            diff.tv_sec = end.tv_sec - start.tv_sec;
            diff.tv_nsec = end.tv_nsec - start.tv_nsec;
            printf("%ld\n", (diff.tv_sec * 1000000000) + diff.tv_nsec);

            wait(NULL);
    }

    return EXIT_SUCCESS;
}