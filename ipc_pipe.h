/*
 * single producer, single consumer channel implemented with pipes
 *
 */

#ifndef IPC_PIPE_H_
#define IPC_PIPE_H_

#include "ipc_runner.h"

struct channel_pipe {
    int send_fds[2], ack_fds[2];

    FILE *send_read, *send_write, *ack_read, *ack_write;
};

extern struct channel_pipe pipe_buf;

void channel_pipe_init(void);
void channel_pipe_child_init(void);  // special for pipe
void channel_pipe_send(int round);
void channel_pipe_recv(int expected_round);

#endif