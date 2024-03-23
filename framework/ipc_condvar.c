#include "ipc_condvar.h"

struct channel_cv *condvar_buf;

void condvar_send(int round) {
    pthread_mutex_lock(&condvar_buf->mutex);

    if (condvar_buf->closed) {
        ERROR("tried to send in closed buffer");
    }

    while (!condvar_buf->empty) pthread_cond_wait(&condvar_buf->cv_full, &condvar_buf->mutex);

    // there is now an empty slot
    condvar_buf->payload = round;
    condvar_buf->empty = false;
    pthread_cond_signal(&condvar_buf->cv_empty);

    pthread_mutex_unlock(&condvar_buf->mutex);
}

void condvar_recv(int expected_round) {
    pthread_mutex_lock(&condvar_buf->mutex);

    while (condvar_buf->empty) {
        if (condvar_buf->closed) ERROR("tried to get in closed buffer");

        pthread_cond_wait(&condvar_buf->cv_empty, &condvar_buf->mutex);
    }

    assert(condvar_buf->payload == expected_round);  // sanity check
    condvar_buf->empty = true;

    pthread_cond_signal(&condvar_buf->cv_full);

    pthread_mutex_unlock(&condvar_buf->mutex);
}