#ifndef IPC_RUNNER_H
#define IPC_RUNNER_H

#define _GNU_SOURCE /* for cpuset functions */
#include <sched.h>

#include <assert.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <unistd.h>

#define NUM_ITER (1000 * 100)
#define NUM_WARMUP 1000

// #define NUM_ITER (100)
// #define NUM_WARMUP (1)

#define ERROR(message)                                                      \
    do {                                                                    \
        fprintf(stderr, "ERROR (%s:%d) %s\n", __FILE__, __LINE__, message); \
        exit(-1);                                                           \
    } while (0)

void check(int ret, const char* errormsg);

// helper functions for serialization
char int_to_byte(int value, int index);
int byte_to_int(int value, int index);

// enum data_mechanism {
//     MMAP, // implemented

//     DATA_NONE
// };

// enum sync_mechanism {
//     CONDITION_VARIABLES,    // implemented
//     MUTEX,
//     SEMAPHORE,
//     ATOMICS,
//     FUTEX,                  // implemented
//     SPINLOCKS,
//     SIGNALS,

//     SYNC_NONE
// };

// enum combined_mechanism {
//     PIPES,
//     SOCKETS,
//     MESSAGE_QUEUES,

//     COMBINED_NONE
// };

// struct benchmark_config {
//     enum data_mechanism data;
//     enum sync_mechanism sync;
//     enum combined_mechanism combined;
// };

#endif
