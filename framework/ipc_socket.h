/*
 * single producer, single consumer channel implemented with UNIX domain sockets
 */

#ifndef IPC_SOCKET_H_
#define IPC_SOCKET_H_

#include "ipc_runner.h"

// extern int socket_fds[2];

void channel_socket_init(void);
void channel_socket_child_init(void);
void channel_socket_send(int round);
void channel_socket_recv(int expected_round);

#endif