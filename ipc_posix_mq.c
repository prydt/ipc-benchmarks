#include "ipc_posix_mq.h"

struct channel_posix_mq posix_mq_buf;

static void init_mq() {
    mq_unlink("/prydtipctest");
    mq_unlink("/prydtipctestack");

    struct mq_attr attr = {.mq_maxmsg = 10, .mq_msgsize = 4};
    posix_mq_buf.pmq = mq_open("/prydtipctest", O_RDWR | O_CREAT, S_IRWXU, &attr); 
    posix_mq_buf.ack = mq_open("/prydtipctestack", O_RDWR | O_CREAT, S_IRWXU, &attr); 
    if (posix_mq_buf.pmq == -1) {
        perror("mq_open");
        exit(-1);
    }
    if (posix_mq_buf.ack == -1) {
        perror("mq_open ack");
        exit(-1);
    }

}
static void send_mq(mqd_t queue, int round) {
    const size_t msglen = sizeof(int) / sizeof(char);
    char msg[msglen];
    for (size_t i = 0; i < msglen; i++) {
        msg[i] = int_to_byte(round, i);
    }

    check(mq_send(queue, msg, msglen, 0), "mq_send");
}

static int recv_mq(mqd_t queue) {
    const size_t msglen = sizeof(int) / sizeof(char);
    char msg[msglen];
    if (mq_receive(queue, msg, msglen, NULL) == -1) {
        perror("mq_receive");
        exit(-1);
    }

    int value = 0;
    for (int i = 0; i < msglen; i++) {
        unsigned char byte = msg[i];
        value += byte_to_int(byte, i);
    }

    return value;
}

static void close_mq(){
    check(mq_close(posix_mq_buf.pmq), "mq_close");
    check(mq_close(posix_mq_buf.ack), "mq_close ack");
}

void channel_posix_mq_init(void) {
    init_mq();
}


void channel_posix_mq_send(int round) {
    send_mq(posix_mq_buf.pmq, round);
    assert(recv_mq(posix_mq_buf.ack) == round);
}

void channel_posix_mq_recv(int expected_round) {
    assert(recv_mq(posix_mq_buf.pmq) == expected_round);
    send_mq(posix_mq_buf.ack, expected_round);
}

void channel_posix_mq_close(void) {
    close_mq();
}