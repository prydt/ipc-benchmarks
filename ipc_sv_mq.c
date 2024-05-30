#include "ipc_sv_mq.h"

struct channel_sv_mq sv_mq_buf;


static void init_mq() {
    sv_mq_buf.msq_id = msgget(IPC_PRIVATE, 0666);
    if (sv_mq_buf.msq_id == -1) {
        perror("msgget");
        exit(-1);
    }
}

static void send_mq(int round, long mtype) {
    struct sv_mg_message msg = {.mtype = mtype, .round = round};
    check(msgsnd(sv_mq_buf.msq_id, &msg, sizeof(msg.round), 0), "msgsnd");
}

static void recv_mq(int expected_round, long mtype) {
    struct sv_mg_message msg;
    if (msgrcv(sv_mq_buf.msq_id, &msg, sizeof(msg.round), mtype, 0) == -1) {
        perror("msgrcv");
        exit(-1);
    }
    assert(expected_round == msg.round);
}

static void close_mq() {
    check(msgctl(sv_mq_buf.msq_id, IPC_RMID, NULL), "msgctl close");
}
/*
static void init_mq() {
    mq_unlink("/prydtipctest");
    // mq_unlink("/prydtipctestack");

    struct mq_attr attr = {.mq_maxmsg = 10, .mq_msgsize = 4};
    sv_mq_buf.pmq = mq_open("/prydtipctest", O_RDWR | O_CREAT, S_IRWXU, &attr); 
    // sv_mq_buf.pmq_ack = mq_open("/prydtipctestack", O_RDWR | O_CREAT, S_IRWXU, &attr); 
    if (sv_mq_buf.pmq == -1) {
        perror("mq_open");
        exit(-1);
    }
    // if (sv_mq_buf.pmq_ack == -1) {
    //     perror("mq_open");
    //     exit(-1);
    // }

}
static void send_mq(int round, long mtype) {
    const size_t msglen = sizeof(int) / sizeof(char);
    char msg[msglen];
    for (size_t i = 0; i < msglen; i++) {
        msg[i] = int_to_byte(round, i);
    }
    printf("send: %d %d %d %d\n", msg[0], msg[1], msg[2], msg[3]);

    check(mq_send(sv_mq_buf.pmq, msg, msglen, 0), "mq_send");
}
static void recv_mq(int expected_round, long mtype ) {
    const size_t msglen = sizeof(int) / sizeof(char);
    char msg[msglen];
    if (mq_receive(sv_mq_buf.pmq, msg, msglen, NULL) == -1) {
        perror("mq_receive");
        exit(-1);
    }

    printf("recv: %d %d %d %d\n", msg[0], msg[1], msg[2], msg[3]);

    int value = 0;
    for (int i = 0; i < msglen; i++) {
        unsigned char byte = msg[i];
        value += byte_to_int(byte, i);
    }
    printf("recv %d == %d\n", value, expected_round);
    assert(value == expected_round);
}

static void close_mq(){
    check(mq_close(sv_mq_buf.pmq), "mq_close");
}
*/

void channel_sv_mq_init(void) {
    init_mq();
}


void channel_sv_mq_send(int round) {
    send_mq(round, 1);
    recv_mq(round, 2);
}

void channel_sv_mq_recv(int expected_round) {
    recv_mq(expected_round, 1);
    send_mq(expected_round, 2);
}

void channel_sv_mq_close(void) {
    close_mq();
}