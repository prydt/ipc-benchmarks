#include <assert.h>
#include <time.h>
#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>

#include <sys/mman.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <pthread.h>
#include <semaphore.h>

#define NUM_ITER (1000 * 10)
#define NUM_WARMUP 1000

void check(int ret, const char* errormsg) {
    if (ret != 0) {
        fprintf(stderr, "error %s\n", errormsg);
        exit(-1);
    }
}