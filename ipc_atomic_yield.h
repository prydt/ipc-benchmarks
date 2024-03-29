/*
 * single producer, single consumer channel implemented with mmap and C11
 * atomics
 */

#ifndef IPC_ATOMIC_YIELD_H_
#define IPC_ATOMIC_YIELD_H_

#include <stdatomic.h>

#include "ipc_runner.h"

struct channel_atomic_yield {
    int payload, ack_payload;

    atomic_bool empty, acked;
};

extern struct channel_atomic_yield *atomic_buf;

void channel_atomic_yield_init(void);
void channel_atomic_yield_send(int round);
void channel_atomic_yield_recv(int expected_round);

#endif