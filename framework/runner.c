#include "ipc_runner.h"
#include "ipc_condvar.h"

void check(int ret, const char* errormsg) {
    if (ret != 0) {
        fprintf(stderr, "ERROR %s, ERR: %s\n", errormsg, strerror(errno));
        exit(-1);
    }
}

void ipc_init(struct benchmark_config *config) {
    if (config->data == MMAP && config->sync == CONDITION_VARIABLES) {
        condvar_buf = (struct channel_cv*)
            mmap(NULL, sizeof(struct channel_cv), PROT_READ | PROT_WRITE,
                 MAP_SHARED | MAP_ANONYMOUS, -1, 0);

        if (condvar_buf == MAP_FAILED) ERROR("mmap failed");

        pthread_mutexattr_t mutex_attr;
        check(pthread_mutexattr_init(&mutex_attr),
              "failed to init mutex attr struct");
        check(pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED),
              "failed to set mutex shared attr");
        check(pthread_mutex_init(&condvar_buf->mutex, &mutex_attr),
              "failed to init mutex");

        pthread_condattr_t cond_attr;
        check(pthread_condattr_init(&cond_attr),
              "failed to init condvar attr struct");
        check(pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED),
              "failed to set condvar shared attr");

        check(pthread_cond_init(&condvar_buf->cv_empty, &cond_attr),
              "failed to init cv_sent");
        check(pthread_cond_init(&condvar_buf->cv_full, &cond_attr),
              "failed to init cv_ack");

        condvar_buf->closed = false;
        condvar_buf->empty = true;
    }
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
    for (int i = 0; i < NUM_ITER; i++) {
        ipc_recv(config, i);
    }
}
void parent_benchmark(struct benchmark_config *config) {
    for (int i = 0; i < NUM_ITER; i++) {
        ipc_send(config, i);
    }
}

int main(int argc, char **argv) {
    // TODO read arguments
    struct benchmark_config cfg = {MMAP, CONDITION_VARIABLES, COMBINED_NONE};

    // TODO argument to set CPU affinity: same core, same hyperthreaded core, different cores
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

            child_warmup(&cfg);
            child_benchmark(&cfg);

            break;
        default:  // parent

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