#include "ipc_condvar.h"

extern void *shared_memory;

void condvar_send(int round) {
    struct channel_cv *shmp = shared_memory;
    pthread_mutex_lock(&shmp->mutex);

    if (shmp->closed) {
        ERROR("tried to send in closed buffer");
    }

    while (!shmp->empty) pthread_cond_wait(&shmp->cv_full, &shmp->mutex);

    // there is now an empty slot
    shmp->payload = round;
    shmp->empty = false;
    pthread_cond_signal(&shmp->cv_empty);

    pthread_mutex_unlock(&shmp->mutex);
}

void condvar_recv(int expected_round) {
    struct channel_cv *shmp = shared_memory;
    pthread_mutex_lock(&shmp->mutex);

    while (shmp->empty) {
        if (shmp->closed) ERROR("tried to get in closed buffer");

        pthread_cond_wait(&shmp->cv_empty, &shmp->mutex);
    }

    assert(shmp->payload == expected_round);  // sanity check
    shmp->empty = true;

    pthread_cond_signal(&shmp->cv_full);

    pthread_mutex_unlock(&shmp->mutex);
}