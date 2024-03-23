/*
 * benchmark function call overhead
 *
 * output format: list of time in nanoseconds, seperated by newline
 *
 */

#include "ipc_bench.h"

struct timespec start, end;

void funct(bool record) {
    // end = clock();
    clock_gettime(CLOCK_MONOTONIC, &end);
    if (record)
        printf("%ld\n", (end.tv_nsec - start.tv_nsec)
                        + (end.tv_sec - start.tv_sec) * 1000000000);

}

void bench(bool record) {
    clock_gettime(CLOCK_MONOTONIC, &start);
    funct(record);
}

// TODO Change to timing whole batch.
int main() {
    for (int i = 0; i < NUM_WARMUP; i++) {
        bench(false);
    }

    for (int i = 0; i < NUM_ITER; i++) {
        bench(true);
    }
}
