/*
 * single producer, single consumer channel implemented with 
 * POSIX Message Queues 
 *
 */

#ifndef IPC_POSIX_MQ_H
#define IPC_POSIX_MQ_H

#include "ipc_runner.h"
#include <fcntl.h>           /* For O_* constants */
#include <sys/stat.h>        /* For mode constants */
#include <mqueue.h>

struct channel_posix_mq {
    mqd_t pmq, ack;
};

extern struct channel_posix_mq posix_mq_buf;

void channel_posix_mq_init(void);
void channel_posix_mq_send(int round);
void channel_posix_mq_recv(int expected_round);
void channel_posix_mq_close(void);
#endif