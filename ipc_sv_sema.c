#include "ipc_sv_sema.h"

struct channel_sv_sema *sv_sema_buf;

void channel_sv_sema_init(void) {
   sv_sema_buf = (struct channel_sv_sema*)mmap(NULL, sizeof(struct channel_sv_sema),
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (sv_sema_buf == MAP_FAILED) ERROR("mmap failed");

    sv_sema_buf->sem_id = semget(IPC_PRIVATE, 1, 0666);
    if (sv_sema_buf->sem_id == -1) {
        perror("semget");
        exit(-1);
    }
    union semun config = {.val = 1};
    check(semctl(sv_sema_buf->sem_id, 0, SETVAL, config), "semctl SETVAL");

    sv_sema_buf->empty = true;
    sv_sema_buf->acked = false;
}

static void sv_lock() {
    struct sembuf op = {.sem_num = 0, .sem_op = -1, .sem_flg = 0};
    check(semop(sv_sema_buf->sem_id, &op, 1), "semop lock");
}
static void sv_unlock() {
    struct sembuf op = {.sem_num = 0, .sem_op = 1, .sem_flg = 0};
    check(semop(sv_sema_buf->sem_id, &op, 1), "semop unlock");
}

void channel_sv_sema_send(int round) {
    sv_lock();
    while (!sv_sema_buf->empty) {
        sv_unlock();
        sv_lock();  // Retry after unlocking
    }

    sv_sema_buf->payload = round;
    sv_sema_buf->empty = false;
    sv_unlock();

    sv_lock();
    while (!sv_sema_buf->acked) {
        sv_unlock();
        sv_lock();  // Retry after unlocking
    }

    assert(sv_sema_buf->ack_payload == round);
    sv_sema_buf->acked = false;
    sv_unlock();
}

void channel_sv_sema_recv(int expected_round) {
    sv_lock();
    while (sv_sema_buf->empty) {
        sv_unlock();
        sv_lock();  // Retry after unlocking
    }

    assert(sv_sema_buf->payload == expected_round);
    sv_sema_buf->empty = true;
    sv_unlock();

    sv_lock();
    while (sv_sema_buf->acked) {
        sv_unlock();
        sv_lock();  // Retry after unlocking
    }

    sv_sema_buf->ack_payload = expected_round;
    sv_sema_buf->acked = true;
    sv_unlock();
}