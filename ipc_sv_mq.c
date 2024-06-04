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
    m_assert(expected_round == msg.round);
}

static void close_mq() {
    check(msgctl(sv_mq_buf.msq_id, IPC_RMID, NULL), "msgctl close");
}

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