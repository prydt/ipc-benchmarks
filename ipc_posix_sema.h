/*
 * single producer, single consumer channel implemented with POSIX Semaphores
 */

#ifndef IPC_POSIX_SEMA_H
#define IPC_POSIX_SEMA_H

#include "ipc_runner.h"
#include <semaphore.h>

struct channel_posix_sema {
    int payload, ack_payload;

    sem_t mutex, ready, ack_ready;
};

extern struct channel_posix_sema *posix_sema_buf;

void channel_posix_sema_init(void);
void channel_posix_sema_send(int round);
void channel_posix_sema_recv(int expected_round);

#endif