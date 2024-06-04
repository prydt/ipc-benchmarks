/*
 * single producer, single consumer channel implemented with System V Semaphores
 */

#ifndef IPC_SV_SEMA_H
#define IPC_SV_SEMA_H

#include "ipc_runner.h"
#include <sys/types.h>
#include <sys/sem.h>
#include <stdio.h>

// Have to define for portability reasons.
// Taken from semctl man pages.
union semun {
    int              val;    /* Value for SETVAL */
    struct semid_ds *buf;    /* Buffer for IPC_STAT, IPC_SET */
    unsigned short  *array;  /* Array for GETALL, SETALL */
#ifdef __linux
    struct seminfo  *__buf;  /* Buffer for IPC_INFO
                                (Linux-specific) */
#endif
};

#define SV_SEMA_MUTEX 0
#define SV_SEMA_READY 1
#define SV_SEMA_READY_ACK 2

struct channel_sv_sema {
    key_t sem_id;
    int payload, ack_payload;
};

extern struct channel_sv_sema *sv_sema_buf;

void channel_sv_sema_init(void);
void channel_sv_sema_send(int round);
void channel_sv_sema_recv(int expected_round);

#endif