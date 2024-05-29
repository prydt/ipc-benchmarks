/*
 * single producer, single consumer channel implemented with 
 * System V Message Queues 
 *
 */

#ifndef IPC_SV_MQ_H
#define IPC_SV_MQ_H

#include "ipc_runner.h"
#include <sys/msg.h>

struct channel_sv_mq {
    key_t msq_id;
};

struct sv_mg_message {
    long mtype; // required, 1 = "send", 2 = "ack" 
    int round;
};

extern struct channel_sv_mq sv_mq_buf;

void channel_sv_mq_init(void);
void channel_sv_mq_send(int round);
void channel_sv_mq_recv(int expected_round);

#endif