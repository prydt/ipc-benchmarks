/*
 * channel (bounded buffer, size=1) for IPC using mmap and pthread condition
 * variables
 */

#ifndef IPC_CONDVAR_H_
#define IPC_CONDVAR_H_

#include "ipc_runner.h"

struct channel_cv {
    pthread_cond_t cv_empty, cv_acked;
    pthread_mutex_t mutex;

    int payload, ack_payload;

    bool empty, acked;
};

extern struct channel_cv *condvar_buf;

void channel_cv_init(void);
void channel_cv_send(int round);
void channel_cv_recv(int expected_round);

#endif