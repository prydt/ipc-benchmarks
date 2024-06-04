#include "ipc_posix_sema.h"

struct channel_posix_sema *posix_sema_buf;

void channel_posix_sema_init(void) {
   posix_sema_buf = (struct channel_posix_sema*) mmap(NULL, sizeof(struct channel_posix_sema),
                                             PROT_READ | PROT_WRITE,
                                             MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (posix_sema_buf == MAP_FAILED) ERROR("mmap failed");

    check(sem_init(&posix_sema_buf->mutex, 1 /* shared between processes*/, 1), "sem_init mutex");
    check(sem_init(&posix_sema_buf->ready, 1 /* shared between processes*/, 0), "sem_init ready");
    check(sem_init(&posix_sema_buf->ack_ready, 1 /* shared between processes*/, 0), "sem_init ack ready");
}

void channel_posix_sema_send(int round) {
    // produce payload
    sem_wait(&posix_sema_buf->mutex);
    posix_sema_buf->payload = round;
    sem_post(&posix_sema_buf->ready);
    sem_post(&posix_sema_buf->mutex);

    // receive ack
    sem_wait(&posix_sema_buf->ack_ready);
    sem_wait(&posix_sema_buf->mutex);
    assert(posix_sema_buf->ack_payload == round);
    sem_post(&posix_sema_buf->mutex);
}

void channel_posix_sema_recv(int expected_round) {
    // receive payload
    sem_wait(&posix_sema_buf->ready);
    sem_wait(&posix_sema_buf->mutex);
    assert(expected_round == posix_sema_buf->payload);

    // produce ack
    posix_sema_buf->ack_payload = posix_sema_buf->payload;
    sem_post(&posix_sema_buf->mutex);
    sem_post(&posix_sema_buf->ack_ready);
}