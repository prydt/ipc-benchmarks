#include "ipc_socket.h"
#include <sys/socket.h>

// pair of file descriptors for both ends of socket
// socket_fds[0] - parent (sender)
// socket_fds[1] - child (receiver)

int socket_fds[2];
FILE *socket_sender, *socket_recver;

void channel_socket_init(void) {

    if (socketpair(AF_UNIX, SOCK_STREAM, 0, socket_fds)) {
        ERROR("failed to set up socketpair");
    }

    socket_sender = fdopen(socket_fds[0], "w");

    // if (socket_sender == NULL || socket_recver == NULL)
        // ERROR("failed to open socket file descriptors");
}


void channel_socket_child_init(void) {
    socket_recver = fdopen(socket_fds[1], "r");
}

void channel_socket_send(int round) {
    //fprintf(socket_sender, "%d\n", round);
    union {
        int number;
        char bytes[sizeof(int) / sizeof(char)];
    } conversion;

    conversion.number = round;

    for (int i = 0; i < sizeof(conversion.bytes) / sizeof(conversion.bytes[0]); i++) {
        putc(conversion.bytes[i], socket_sender);
    }

    fflush(socket_sender);
}

void channel_socket_recv(int expected_round) {
    /* int round; */
    //fscanf(socket_recver, "%d", &round);
    //

    union {
        int number;
        char bytes[sizeof(int) / sizeof(char)];
    } conversion;

    /* conversion.number = round; */

    for (int i = 0; i < sizeof(conversion.bytes) / sizeof(conversion.bytes[0]); i++) {
        /* putc(conversion.bytes[i], socket_sender); */
        conversion.bytes[i] = (char)fgetc(socket_recver);
    }


    // printf("%d = %d\n", round, expected_round);
    assert(conversion.number == expected_round);
}
