#include "ipc_atomic_spin.h"

struct channel_atomic_spin *atomic_spin_buf;

void channel_atomic_spin_init(void) {
    atomic_spin_buf = (struct channel_atomic_spin *)mmap(
        NULL, sizeof(struct channel_atomic_spin), PROT_READ | PROT_WRITE,
        MAP_SHARED | MAP_ANONYMOUS, -1, 0);

    if (atomic_spin_buf == MAP_FAILED) ERROR("mmap failed");

    atomic_spin_buf->empty = true;
    atomic_spin_buf->acked = false;
}

static inline void wait_while_atomic_is(atomic_bool *val, bool current) {
    while (true) {
        while (atomic_load_explicit(val, memory_order_relaxed) == current) {
            // spin
            _mm_pause();
        }

        if (atomic_load_explicit(val, memory_order_acquire) != current)
            break;
    }
}

void channel_atomic_spin_send(int round) {
    wait_while_atomic_is(&atomic_spin_buf->empty, false);
    atomic_spin_buf->payload = round;
    atomic_store_explicit(&atomic_spin_buf->empty, false, memory_order_seq_cst);

    // wait for ack
    wait_while_atomic_is(&atomic_spin_buf->acked, false);
    assert(atomic_spin_buf->ack_payload == round);
    atomic_store_explicit(&atomic_spin_buf->acked, false, memory_order_seq_cst);
}

void channel_atomic_spin_recv(int expected_round) {
    // while (atomic_load_explicit(&atomic_buf->empty, memory_order_seq_cst))
    //     sched_yield();
    wait_while_atomic_is(&atomic_spin_buf->empty, true);
    assert(atomic_spin_buf->payload == expected_round);
    atomic_store_explicit(&atomic_spin_buf->empty, true, memory_order_seq_cst);

    // // do ack
    atomic_spin_buf->ack_payload = atomic_spin_buf->payload;
    atomic_store_explicit(&atomic_spin_buf->acked, true, memory_order_seq_cst);
}