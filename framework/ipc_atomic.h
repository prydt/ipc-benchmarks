/*
 * single producer, single consumer channel implemented with mmap and C11
 * atomics
 */

#ifndef IPC_ATOMIC_H
#define IPC_ATOMIC_H

#include <stdatomic.h>
#include <threads.h>

#include "ipc_runner.h"

struct channel_atomic {
    int payload;

    atomic_bool empty;
};

extern struct channel_atomic *atomic_buf;

void channel_atomic_init(void);
void channel_atomic_send(int round);
void channel_atomic_recv(int expected_round);

#endif