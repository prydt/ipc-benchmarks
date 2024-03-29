/*
 * single producer, single consumer channel implemented with mmap and futexes
 */

#ifndef IPC_FUTEX_H_
#define IPC_FUTEX_H_

#include <linux/futex.h>
#include <sys/syscall.h>

#include "ipc_runner.h"

struct channel_futex {
    int payload, ack_payload;

    uint32_t empty, acked;
};

extern struct channel_futex *futex_buf;

void channel_futex_init(void);
void channel_futex_send(int round);
void channel_futex_recv(int expected_round);

#endif