#include "ipc_sv_sema.h"

struct channel_sv_sema *sv_sema_buf;

void channel_sv_sema_init(void) {
   sv_sema_buf = (struct channel_sv_sema*)mmap(NULL, sizeof(struct channel_sv_sema),
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (sv_sema_buf == MAP_FAILED) ERROR("mmap failed");

    sv_sema_buf->sem_id = semget(IPC_PRIVATE, 3, 0666); // 0 = mutex, 1 = ready
    if (sv_sema_buf->sem_id == -1) {
        perror("semget");
        exit(-1);
    }
    union semun config = {.val = 1};
    check(semctl(sv_sema_buf->sem_id, SV_SEMA_MUTEX, SETVAL, config), "semctl SETVAL");
    config.val = 0;
    check(semctl(sv_sema_buf->sem_id, SV_SEMA_READY, SETVAL, config), "semctl SETVAL READY");
    config.val = 0;
    check(semctl(sv_sema_buf->sem_id, SV_SEMA_READY_ACK, SETVAL, config), "semctl SETVAL READY ACK");

    sv_sema_buf->empty = true;
    sv_sema_buf->acked = false;
}

static void sv_wait(int semnum) {
    struct sembuf op = {.sem_num = semnum, .sem_op = -1, .sem_flg = 0};
    check(semop(sv_sema_buf->sem_id, &op, 1), "semop lock");
}
static void sv_signal(int semnum) {
    struct sembuf op = {.sem_num = semnum, .sem_op = 1, .sem_flg = 0};
    check(semop(sv_sema_buf->sem_id, &op, 1), "semop unlock");
}

void channel_sv_sema_send(int round) {
    // produce payload
    sv_wait(SV_SEMA_MUTEX);
    sv_sema_buf->payload = round;
    sv_signal(SV_SEMA_READY);
    sv_signal(SV_SEMA_MUTEX);

    // receive ack
    sv_wait(SV_SEMA_READY_ACK);
    sv_wait(SV_SEMA_MUTEX);
    assert(sv_sema_buf->ack_payload == round);
    sv_signal(SV_SEMA_MUTEX);
}

void channel_sv_sema_recv(int expected_round) {
    // receive payload
    sv_wait(SV_SEMA_READY);
    sv_wait(SV_SEMA_MUTEX);
    assert(expected_round == sv_sema_buf->payload);

    // produce ack
    sv_sema_buf->ack_payload = sv_sema_buf->payload;
    sv_signal(SV_SEMA_MUTEX);
    sv_signal(SV_SEMA_READY_ACK);
}