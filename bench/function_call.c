/*
 * benchmark function call overhead
 *
 * output format: list of time in seconds, seperated by newline
 *
 */

#include <time.h>
#include <stdio.h>
#include <stdbool.h>

#define NUM_ITER (1000 * 10)
#define NUM_WARMUP 1000

clock_t start, end;

void funct(char *payload) {
    end = clock();
}

void bench(bool record) {
    start = clock();
    funct(NULL);
    if (record) {
        printf("%Lf\n", (long double) (end - start) / CLOCKS_PER_SEC);
        //printf("%Ld\n", (end - start) );
    }
}

int main() {

    for (int i = 0; i < NUM_WARMUP; i++) {
        bench(false);
    }

    for (int i = 0; i < NUM_ITER; i++) {
        bench(true);
    }
}
