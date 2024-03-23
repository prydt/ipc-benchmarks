/*
 * IPC socket overhead
 *
 * output format: list of time in nanoseconds, seperated by newline
 *
 */

#include "ipc_bench.h"

char buffer[128];

void local_send(int sockfd) {
    struct timespec start;
    clock_gettime(CLOCK_MONOTONIC, &start);
    long start_time = start.tv_nsec + start.tv_sec * 1000000000;
    size_t len = sprintf(buffer, "%ld\n", start_time);
    // printf("PARENT: sending %ld = %s\n", start_time, buffer);
    send(sockfd, buffer, len, 0);
}

void local_recv(int sockfd, bool record) {
    struct timespec end;
    long start_time;

    ssize_t amt_read = recv(sockfd, buffer, 128, 0);
    if (amt_read == -1) {
        printf("failed to recv: %s\n", strerror(errno));
        exit(-1);
    }
    clock_gettime(CLOCK_MONOTONIC, &end);
    buffer[amt_read] = '\0';
    sscanf(buffer, "%ld", &start_time);
    // printf("CHILD: got (amt-read=%d) %s = %ld\n",amt_read, buffer, start_time);


    long elapsed_time = ((end.tv_sec * 1000000000) + end.tv_nsec) - start_time;
    if (record)
        printf("%ld\n", elapsed_time);
}

int main() {

    struct sockaddr_un addr;        
    int sockfd, cfd;

    switch(fork()) {
    case -1:
        fprintf(stderr, "failed to fork()\n");
        exit(-1);

    case 0: // child (Server)

        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        // check(sockfd != -1, "child: failed to create socket");
        if (sockfd == -1) {
            printf("child: failed to make socket\n");
            exit(-1);
        }

        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, "BENCHSOCKET", sizeof(addr.sun_path) - 1);
        addr.sun_path[0] = '\0';


        check(bind(sockfd, (struct sockaddr*) &addr, sizeof(struct sockaddr_un)), "child: failed to bind");
        check(listen(sockfd, 1), "child: failed to listen");

        cfd = accept(sockfd, NULL, NULL);

        // for (int i = 0; i < NUM_WARMUP; i++)
        //     local_recv(cfd, false);

        // for (int i = 0; i < NUM_ITER; i++)
        //     local_recv(cfd, true);
        local_recv(cfd, true);
        local_recv(cfd, true);


        break;

    default: // parent (Client)

        sockfd = socket(AF_UNIX, SOCK_STREAM, 0);
        // check(sockfd != -1, "parent: failed to create socket");
        if (sockfd == -1) {
            printf("parent: failed to make socket\n");
            exit(-1);
        }


        memset(&addr, 0, sizeof(struct sockaddr_un));
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, "BENCHSOCKET", sizeof(addr.sun_path) - 1);
        addr.sun_path[0] = '\0';

        sleep(1); // TODO maybe make something less brittle?
        check(connect(sockfd, (struct sockaddr*) &addr, sizeof(struct sockaddr_un)), "parent: failed to connect");

        // for (int i = 0; i < NUM_WARMUP; i++)
        //     local_send(sockfd);

        // for (int i = 0; i < NUM_ITER; i++)
        //     local_send(sockfd);
        local_send(sockfd);
        local_send(sockfd);

        wait(NULL);
    }
}