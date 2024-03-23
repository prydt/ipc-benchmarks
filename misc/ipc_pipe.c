/*
 * IPC pipe overhead
 *
 * output format: list of time in nanoseconds, seperated by newline
 *
 */

#include "ipc_bench.h"

// reading = 0, writing = 1
int send_fds[2],
    ack_fds[2];

FILE *send_read, *send_write,
     *ack_read, *ack_write;

struct timespec start, end;

void send() {
    clock_gettime(CLOCK_MONOTONIC, &start);
    fprintf(send_write, "%ld\n", start.tv_nsec + start.tv_sec * 1000000000);
    fflush(send_write);

    // Wait until ack.
    fgetc(ack_read);
}

void recv(bool record) {
    long ns_time;
    fscanf(send_read, "%ld", &ns_time);
    clock_gettime(CLOCK_MONOTONIC, &end);

    long time_diff = (end.tv_sec * 1000000000 + end.tv_nsec) - ns_time;
    if (record)
        printf("%ld\n", time_diff);

    // Ack.
    fputc('A', ack_write);
    fflush(ack_write);
}

int main() {


    check(pipe(send_fds), "failed to create send pipe");
    check(pipe(ack_fds), "failed to create ack pipe");

    switch(fork()) {
    case -1:
        fprintf(stderr, "failed to fork()\n");
        exit(-1);

    case 0: // child

        // close writing fd for send pipe
        close(send_fds[1]);

        send_read = fdopen(send_fds[0], "r");
        ack_write = fdopen(ack_fds[1], "w");

       for (int i = 0; i < NUM_WARMUP; i++)
            recv(false);

        for (int i = 0; i < NUM_ITER; i++)
            recv(true);


        break;

    default: // parent

        // close writing fd for ack pipe
        close(ack_fds[1]);

        send_write = fdopen(send_fds[1], "w");
        ack_read = fdopen(ack_fds[0], "r");

        for (int i = 0; i < NUM_WARMUP; i++)
            send();

        for (int i = 0; i < NUM_ITER; i++)
            send();

        wait(NULL);
    }
}