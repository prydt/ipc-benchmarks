#include "ipc_condvar.h"

struct channel_cv *condvar_buf;

void channel_cv_init() {
    condvar_buf = (struct channel_cv *)mmap(NULL, sizeof(struct channel_cv),
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (condvar_buf == MAP_FAILED) ERROR("mmap failed");

    pthread_mutexattr_t mutex_attr;
    check(pthread_mutexattr_init(&mutex_attr),
          "failed to init mutex attr struct");
    check(pthread_mutexattr_setpshared(&mutex_attr, PTHREAD_PROCESS_SHARED),
          "failed to set mutex shared attr");
    check(pthread_mutex_init(&condvar_buf->mutex, &mutex_attr),
          "failed to init mutex");

    pthread_condattr_t cond_attr;
    check(pthread_condattr_init(&cond_attr),
          "failed to init condvar attr struct");
    check(pthread_condattr_setpshared(&cond_attr, PTHREAD_PROCESS_SHARED),
          "failed to set condvar shared attr");

    check(pthread_cond_init(&condvar_buf->cv_empty, &cond_attr),
          "failed to init cv_empty");
    check(pthread_cond_init(&condvar_buf->cv_acked, &cond_attr),
          "failed to init cv_acked");

    condvar_buf->empty = true;
    condvar_buf->acked = false;
}


void channel_cv_send(int round) {
    pthread_mutex_lock(&condvar_buf->mutex);

    while (!condvar_buf->empty)
        pthread_cond_wait(&condvar_buf->cv_acked, &condvar_buf->mutex);

    // there is now an empty slot
    condvar_buf->payload = round;
    condvar_buf->empty = false;
    pthread_cond_signal(&condvar_buf->cv_empty);

    while(!condvar_buf->acked) {
        pthread_cond_wait(&condvar_buf->cv_acked, &condvar_buf->mutex);
    }

    // recieved ack
    m_assert(condvar_buf->ack_payload == round);
    condvar_buf->acked = false;
    pthread_mutex_unlock(&condvar_buf->mutex);
}

void channel_cv_recv(int expected_round) {
    pthread_mutex_lock(&condvar_buf->mutex);

    while (condvar_buf->empty) {
        pthread_cond_wait(&condvar_buf->cv_empty, &condvar_buf->mutex);
    }

    m_assert(condvar_buf->payload == expected_round);  // sanity check
    condvar_buf->empty = true;

    condvar_buf->ack_payload = condvar_buf->payload;
    condvar_buf->acked = true;
    pthread_cond_signal(&condvar_buf->cv_acked);
    pthread_mutex_unlock(&condvar_buf->mutex);
}