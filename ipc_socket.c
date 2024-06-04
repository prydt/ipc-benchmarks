#include "ipc_socket.h"

#include <sys/socket.h>

// pair of file descriptors for both ends of socket
// socket_fds[0] - parent (sender)
// socket_fds[1] - child (receiver)

int socket_fds[2], ack_fds[2];
// FILE *socket_sender, *socket_recver;
// FILE *parent_sender, *parent_recver,
//      *child_sender, *child_recver;
FILE *socket_sender, *socket_recver, *ack_sender, *ack_recver;

void channel_socket_init(void) {
    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds)) {
        ERROR("failed to set up socketpair");
    }

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, ack_fds)) {
        ERROR("failed to set up ack socketpair");
    }

    socket_sender = fdopen(socket_fds[0], "w");
    socket_recver = fdopen(socket_fds[1], "r");
    ack_sender = fdopen(ack_fds[0], "w");
    ack_recver = fdopen(ack_fds[1], "r");

    if (socket_sender == NULL || socket_recver == NULL)
        ERROR("failed to open main socket fd");

    if (ack_sender == NULL || ack_recver == NULL)
        ERROR("failed to open ack socket fd");
}

void send_int_over_socket(int value, FILE *sender) {
    // serialize int to char
    for (int i = 0; i < sizeof(int) / sizeof(char); i++) {
        char byte = int_to_byte(value, i);
        putc(byte, sender);
    }

    fflush(sender);
}

void recv_int_over_socket(int expected_value, FILE *recver) {
    // decode int from chars
    int value = 0;
    for (int i = 0; i < (sizeof(int) / sizeof(char)); i++) {
        int byte = fgetc(recver);
        value += byte_to_int(byte, i);
    }

    m_assert(value == expected_value);
}

void channel_socket_send(int round) {
    send_int_over_socket(round, socket_sender);
    // wait for ack
    recv_int_over_socket(round, ack_recver);
}

void channel_socket_recv(int expected_round) {
    recv_int_over_socket(expected_round, socket_recver);
    // send ack
    send_int_over_socket(expected_round, ack_sender);
}
