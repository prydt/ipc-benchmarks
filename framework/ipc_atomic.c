#include "ipc_atomic.h"

struct channel_atomic *atomic_buf;

void channel_atomic_init(void) {
    atomic_buf = (struct channel_atomic *)mmap(
        NULL, sizeof(struct channel_atomic), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (atomic_buf == MAP_FAILED) ERROR("mmap failed");

    atomic_buf->empty = true;
}

void channel_atomic_send(int round) {
    while (!atomic_load_explicit(&atomic_buf->empty,
                                 memory_order_seq_cst))  // wait until empty
        thrd_yield();

    atomic_buf->payload = round;
    atomic_store_explicit(&atomic_buf->empty, false, memory_order_seq_cst);
}

void channel_atomic_recv(int expected_round) {
    while (atomic_load_explicit(&atomic_buf->empty, memory_order_seq_cst))
        thrd_yield();

    assert(atomic_buf->payload == expected_round);
    atomic_store_explicit(&atomic_buf->empty, true, memory_order_seq_cst);
}