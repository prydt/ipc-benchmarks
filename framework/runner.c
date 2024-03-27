#include "ipc_runner.h"

#include "ipc_atomic_yield.h"
#include "ipc_condvar.h"
#include "ipc_futex.h"
#include "ipc_pipe.h"
#include "ipc_socket.h"

// #define IPC_CONDVAR_BENCH
// #define IPC_FUTEX_BENCH
// #define IPC_ATOMIC_BENCH
// #define IPC_PIPE_BENCH
#define IPC_SOCKET_BENCH

void check(int ret, const char *errormsg) {
    if (ret != 0) {
        fprintf(stderr, "ERROR %s, ERR: %s\n", errormsg, strerror(errno));
        exit(-1);
    }
}

void ipc_init() {
#ifdef IPC_CONDVAR_BENCH
    channel_cv_init();
#endif

#ifdef IPC_FUTEX_BENCH
    channel_futex_init();
#endif

#ifdef IPC_ATOMIC_BENCH
    channel_atomic_init();
#endif

#ifdef IPC_PIPE_BENCH
    channel_pipe_init();
#endif

#ifdef IPC_SOCKET_BENCH
    channel_socket_init();
#endif
}

void ipc_send(int round) {
#ifdef IPC_CONDVAR_BENCH
    channel_cv_send(round);
#endif

#ifdef IPC_FUTEX_BENCH
    channel_futex_send(round);
#endif

#ifdef IPC_ATOMIC_BENCH
    channel_atomic_send(round);
#endif

#ifdef IPC_PIPE_BENCH
    channel_pipe_send(round);
#endif

#ifdef IPC_SOCKET_BENCH
    channel_socket_send(round);
#endif
}

void ipc_recv(int expected_round) {
#ifdef IPC_CONDVAR_BENCH
    channel_cv_recv(expected_round);
#endif

#ifdef IPC_FUTEX_BENCH
    channel_futex_recv(expected_round);
#endif

#ifdef IPC_ATOMIC_BENCH
    channel_atomic_recv(expected_round);
#endif

#ifdef IPC_PIPE_BENCH
    channel_pipe_recv(expected_round);
#endif

#ifdef IPC_SOCKET_BENCH
    channel_socket_recv(expected_round);
#endif
}

void child_warmup() {
    for (int i = 0; i < NUM_WARMUP; i++) {
        ipc_recv(i);
    }
}
void parent_warmup() {
    for (int i = 0; i < NUM_WARMUP; i++) {
        ipc_send(i);
    }
}

void child_benchmark() {
    for (int i = 0; i < NUM_ITER; i++) {
        ipc_recv(i);
    }
}
void parent_benchmark() {
    for (int i = 0; i < NUM_ITER; i++) {
        ipc_send(i);
    }
}

int main(int argc, char **argv) {
    // TODO read arguments
    // struct benchmark_config cfg = {MMAP, CONDITION_VARIABLES, COMBINED_NONE};

    // TODO argument to set CPU affinity: same core, same hyperthreaded core,
    // different cores
    cpu_set_t cpuset;
    CPU_ZERO(&cpuset);
    CPU_SET(0, &cpuset);
    check(pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset),
          "failed to set CPU affinity");

    const long PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    ipc_init();

    switch (fork()) {
        case -1:  // FAIL
            ERROR("failed to fork()");

        case 0:  // child
            #ifdef IPC_PIPE_BENCH
                // need to close one end of pipe
                channel_pipe_child_init();
            #endif

            child_warmup();
            child_benchmark();

            break;
        default:  // parent

            parent_warmup();

            struct timespec start, end, diff;
            clock_gettime(CLOCK_MONOTONIC, &start);

            parent_benchmark();

            clock_gettime(CLOCK_MONOTONIC, &end);
            diff.tv_sec = end.tv_sec - start.tv_sec;
            diff.tv_nsec = end.tv_nsec - start.tv_nsec;
            printf("%ld\n", (diff.tv_sec * 1000000000) + diff.tv_nsec);

            wait(NULL);
    }

    return EXIT_SUCCESS;
}
