#include "ipc_pipe.h"

struct channel_pipe pipe_buf;

void channel_pipe_init(void) {
    check(pipe(pipe_buf.send_fds), "failed to create send pipe");
    check(pipe(pipe_buf.ack_fds), "failed to create ack pipe");
    pipe_buf.send_write = fdopen(pipe_buf.send_fds[1], "w");
    pipe_buf.ack_read = fdopen(pipe_buf.ack_fds[0], "r");
}

void channel_pipe_child_init(void) {
    close(pipe_buf.send_fds[1]);
    close(pipe_buf.ack_fds[0]);
    pipe_buf.send_read = fdopen(pipe_buf.send_fds[0], "r");
    pipe_buf.ack_write = fdopen(pipe_buf.ack_fds[1], "w");
}

static inline void pipe_send(int value, FILE *sendpipe) {
    for (int i = 0; i < sizeof(int) / sizeof(char); i++) {
        char byte = int_to_byte(value, i);
        putc(byte,sendpipe);
    }

    fflush(sendpipe);
}

static inline void pipe_recv(int expected_value, FILE *recvpipe) {
    int value = 0;
    for (int i = 0; i < sizeof(int) / sizeof(char); i++) {
        int byte = fgetc(recvpipe);
        value += byte_to_int(byte, i);
    }
    
    m_assert(value == expected_value);
}

void channel_pipe_send(int round) {
    pipe_send(round, pipe_buf.send_write);
    // wait for ack
    pipe_recv(round, pipe_buf.ack_read);
}

void channel_pipe_recv(int expected_round) {
    pipe_recv(expected_round, pipe_buf.send_read);
    // send ack
    pipe_send(expected_round, pipe_buf.ack_write);
}