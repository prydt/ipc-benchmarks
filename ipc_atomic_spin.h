/*
 * single producer, single consumer channel implemented with mmap and C11
 * atomics (busy loops)
 */

#ifndef IPC_ATOMIC_SPIN_H_
#define IPC_ATOMIC_SPIN_H_

#include "ipc_runner.h"
#include <stdatomic.h>
#include <immintrin.h> // for _mm_pause

struct channel_atomic_spin {
    volatile int payload, ack_payload;

    atomic_bool empty, acked;
};

extern struct channel_atomic_spin *atomic_spin_buf;

void channel_atomic_spin_init(void);
void channel_atomic_spin_send(int round);
void channel_atomic_spin_recv(int expected_round);

#endif