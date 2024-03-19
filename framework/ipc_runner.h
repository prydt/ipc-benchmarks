#define _GNU_SOURCE /* for cpuset functions */

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define ERROR(message)                                                      \
    do {                                                                    \
        fprintf(stderr, "ERROR (%s:%d) %s\n", __FILE__, __LINE__, message); \
        exit(-1);                                                           \
    } while (0)

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
    enum data_mechanism data;
    enum sync_mechanism sync;
    enum combined_mechanism combined;
};
