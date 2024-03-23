/*
 * channel (bounded buffer, size=1) for IPC using mmap and pthread condition
 * variables
 */

#ifndef IPC_CONDVAR_H
#define IPC_CONDVAR_H

#include "ipc_runner.h"

struct channel_cv {
    pthread_cond_t cv_empty, cv_full;
    pthread_mutex_t mutex;

    int payload;

    bool closed, empty;
};

extern struct channel_cv *condvar_buf;

void channel_cv_init(void);
void channel_cv_send(int round);
void channel_cv_recv(int expected_round);

#endif