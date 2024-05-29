#include "ipc_runner.h"

#include "ipc_atomic_spin.h"
#include "ipc_atomic_yield.h"
#include "ipc_condvar.h"
#include "ipc_futex.h"
#include "ipc_pipe.h"
#include "ipc_socket.h"
#include "ipc_sv_mq.h"

void ipc_init() {
#ifdef IPC_CONDVAR_BENCH
    channel_cv_init();
#endif

#ifdef IPC_FUTEX_BENCH
    channel_futex_init();
#endif

#ifdef IPC_ATOMIC_YIELD_BENCH
    channel_atomic_yield_init();
#endif

#ifdef IPC_ATOMIC_SPIN_BENCH
    channel_atomic_spin_init();
#endif

#ifdef IPC_PIPE_BENCH
    channel_pipe_init();
#endif

#ifdef IPC_SOCKET_BENCH
    channel_socket_init();
#endif

#ifdef IPC_SV_MQ_BENCH
    channel_sv_mq_init();
#endif
}

void ipc_send(int round) {
#ifdef IPC_CONDVAR_BENCH
    channel_cv_send(round);
#endif

#ifdef IPC_FUTEX_BENCH
    channel_futex_send(round);
#endif

#ifdef IPC_ATOMIC_YIELD_BENCH
    channel_atomic_yield_send(round);
#endif

#ifdef IPC_ATOMIC_SPIN_BENCH
    channel_atomic_spin_send(round);
#endif

#ifdef IPC_PIPE_BENCH
    channel_pipe_send(round);
#endif

#ifdef IPC_SOCKET_BENCH
    channel_socket_send(round);
#endif

#ifdef IPC_SV_MQ_BENCH
    channel_sv_mq_send(round);
#endif
}

void ipc_recv(int expected_round) {
#ifdef IPC_CONDVAR_BENCH
    channel_cv_recv(expected_round);
#endif

#ifdef IPC_FUTEX_BENCH
    channel_futex_recv(expected_round);
#endif

#ifdef IPC_ATOMIC_YIELD_BENCH
    channel_atomic_yield_recv(expected_round);
#endif

#ifdef IPC_ATOMIC_SPIN_BENCH
    channel_atomic_spin_recv(expected_round);
#endif

#ifdef IPC_PIPE_BENCH
    channel_pipe_recv(expected_round);
#endif

#ifdef IPC_SOCKET_BENCH
    channel_socket_recv(expected_round);
#endif

#ifdef IPC_SV_MQ_BENCH
    channel_sv_mq_recv(expected_round);
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

// for testing
void print_config() {
#ifdef IPC_CONDVAR_BENCH
    printf("condvar ");
#endif

#ifdef IPC_FUTEX_BENCH
    printf("futex ");
#endif

#ifdef IPC_ATOMIC_YIELD_BENCH
    printf("atomic_yield ");
#endif

#ifdef IPC_ATOMIC_SPIN_BENCH
    printf("atomic_spin ");
#endif

#ifdef IPC_PIPE_BENCH
    printf("pipe ");
#endif

#ifdef IPC_SOCKET_BENCH
    printf("socket ");
#endif

#ifdef IPC_SV_MQ_BENCH
    printf("System V Message Queue ");
#endif



#ifdef IPC_CPU_SAME
    printf("cpu same\n");
#endif
#ifdef IPC_CPU_HYPERTHREAD
    printf("cpu hyperthread\n");
#endif
#ifdef IPC_CPU_DIFFERENT
    printf("cpu different\n");
#endif
}

void print_cpu_set(cpu_set_t *cpusetp, const char *name) {
    printf("%s: ",name);
    int num_cores = (int) sysconf(_SC_NPROCESSORS_ONLN);
    for (int i = 0; i < num_cores; i++) {
        if (CPU_ISSET(i, cpusetp)) {
            printf("core %d, ", i);
        }
    }
    printf("\n");
}

int main(int argc, char **argv) {
    // print_config();

    cpu_set_t cpuset, other_cpuset;
    CPU_ZERO(&cpuset);
    CPU_ZERO(&other_cpuset);
    CPU_SET(0, &cpuset);

    check(pthread_setaffinity_np(pthread_self(), sizeof(cpuset), &cpuset),
          "failed to set CPU affinity");
    // print_cpu_set(&cpuset, "parent");
    
    // const long PAGE_SIZE = sysconf(_SC_PAGE_SIZE);

    ipc_init();

    switch (fork()) {
        case -1:  // FAIL
            ERROR("failed to fork()");

        case 0:  // child
            #ifdef IPC_CPU_SAME
                CPU_SET(0, &other_cpuset);
                check(pthread_setaffinity_np(pthread_self(), sizeof(other_cpuset), &other_cpuset),
                    "failed to set CPU affinity");
                // print_cpu_set(&other_cpuset, "child");
            #endif

            #ifdef IPC_CPU_HYPERTHREAD
                // CPU 0 and 1 are hyperthreaded (siblings)
                CPU_SET(1, &other_cpuset);
                check(pthread_setaffinity_np(pthread_self(), sizeof(other_cpuset), &other_cpuset),
                    "failed to set CPU affinity");
                // print_cpu_set(&other_cpuset, "child");
            #endif

            #ifdef IPC_CPU_DIFFERENT
                // CPU 0 and 2 are different cores
                CPU_SET(2, &other_cpuset);
                check(pthread_setaffinity_np(pthread_self(), sizeof(other_cpuset), &other_cpuset),
                    "failed to set CPU affinity");
                // print_cpu_set(&other_cpuset, "child");
            #endif


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
