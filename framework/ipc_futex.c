#include "ipc_futex.h"
#include <linux/futex.h>
#include <sys/syscall.h>

struct channel_futex *futex_buf;

static long atomic_wait(uint32_t *futex, uint32_t current_val);
static long atomic_notify(uint32_t *futex, uint32_t amt_to_wake);

inline long atomic_wait(uint32_t *futex, uint32_t current_val) {
    return syscall(SYS_futex, futex, FUTEX_WAIT, current_val, NULL, NULL, 0);
}

inline long atomic_notify(uint32_t *futex, uint32_t amt_to_wake) {
    return syscall(SYS_futex, futex, FUTEX_WAKE, amt_to_wake, NULL, NULL, 0);
}

void channel_futex_init(void) {
    futex_buf = (struct channel_futex*)mmap(NULL, sizeof(struct channel_futex),
                                            PROT_READ | PROT_WRITE,
                                            MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (futex_buf == MAP_FAILED) ERROR("mmap failed"); 

    futex_buf->empty = 1;
}

void channel_futex_send(int round) {
    while (!futex_buf->empty)
        atomic_wait(&futex_buf->empty, 0); // wait while empty == 0 (still full)

    futex_buf->payload = round;
    futex_buf->empty = 0; /*not empty*/
    atomic_notify(&futex_buf->empty, 1);
}

void channel_futex_recv(int expected_round) {
    while (futex_buf->empty)
        atomic_wait(&futex_buf->empty, 1); // wait while empty == 1 (still empty)

    assert(futex_buf->payload == expected_round);
    futex_buf->empty = 1; /*set to empty*/
    atomic_notify(&futex_buf->empty, 1);
}