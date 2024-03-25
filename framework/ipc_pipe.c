#include "ipc_pipe.h"

struct channel_pipe pipe_buf;

void channel_pipe_init(void) {
    check(pipe(pipe_buf.send_fds), "failed to create send pipe");
    // check(pipe(pipe_buf.ack_fds), "failed to create ack pipe");
    pipe_buf.send_write = fdopen(pipe_buf.send_fds[1], "w");
}

void channel_pipe_child_init(void) {
    close(pipe_buf.send_fds[1]);
    pipe_buf.send_read = fdopen(pipe_buf.send_fds[0], "r");
}

void channel_pipe_send(int round) {
    // fprintf(pipe_buf.send_write, "%d\n", round);

    union {
        int number;
        char bytes[sizeof(int) / sizeof(char)];
    } conversion;

    conversion.number = round;

    for (int i = 0; i < sizeof(conversion.bytes) / sizeof(conversion.bytes[0]); i++) {
        putc(conversion.bytes[i], pipe_buf.send_write);
    }

    fflush(pipe_buf.send_write);
}

void channel_pipe_recv(int expected_round) {
    // int recv_round;
    // fscanf(pipe_buf.send_read, "%d", &recv_round);

    union {
        int number;
        char bytes[sizeof(int) / sizeof(char)];
    } conversion;

    for (int i = 0; i < sizeof(conversion.bytes) / sizeof(conversion.bytes[0]); i++) {
        conversion.bytes[i] = fgetc(pipe_buf.send_read);
    }
    
    assert(conversion.number == expected_round);
}