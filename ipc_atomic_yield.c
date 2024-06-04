#include "ipc_atomic_yield.h"

struct channel_atomic_yield *atomic_buf;

void channel_atomic_yield_init(void) {
    atomic_buf = (struct channel_atomic_yield *)mmap(
        NULL, sizeof(struct channel_atomic_yield), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (atomic_buf == MAP_FAILED) ERROR("mmap failed");

    atomic_buf->empty = true;
    atomic_buf->acked = false;
}

void channel_atomic_yield_send(int round) {
    // TODO maybe replace with proper spinning?
    while (!atomic_load_explicit(&atomic_buf->empty,
                                 memory_order_seq_cst))  // wait until empty
        sched_yield();
        

    atomic_buf->payload = round;
    atomic_store_explicit(&atomic_buf->empty, false, memory_order_seq_cst);

    // wait for ack
    while (!atomic_load_explicit(&atomic_buf->acked,
                                 memory_order_seq_cst))  // wait until empty
        sched_yield();

    m_assert(atomic_buf->ack_payload == round);
    atomic_store_explicit(&atomic_buf->acked, false, memory_order_seq_cst);
}

void channel_atomic_yield_recv(int expected_round) {
    while (atomic_load_explicit(&atomic_buf->empty, memory_order_seq_cst))
        sched_yield();

    m_assert(atomic_buf->payload == expected_round);
    atomic_store_explicit(&atomic_buf->empty, true, memory_order_seq_cst);

    // do ack
    atomic_buf->ack_payload = atomic_buf->payload;
    atomic_store_explicit(&atomic_buf->acked, true, memory_order_seq_cst);
}