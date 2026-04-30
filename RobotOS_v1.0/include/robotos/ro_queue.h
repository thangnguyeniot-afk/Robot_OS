/* ============================================================================
 * ro_queue.h — RobotOS Fixed-Size Message Queue
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming technique: Bounded FIFO with caller-provided backing buffer.
 *
 * Data structure: Fixed-size ring buffer.
 *   - Head and tail indices wrap modulo capacity
 *   - Each slot is `item_size` bytes — items are memcpy'd in/out
 *   - ISR-safe variants (_isr) disable interrupts briefly on Zephyr
 *   - Thread-safe send/recv use kernel semaphores for blocking
 *
 * Memory discipline: The caller provides all backing memory.
 *   - No malloc, no heap, no Adapter global slab
 *   - Buffer size MUST be >= item_size * capacity
 *
 * RO_QUEUE_WAIT_FOREVER: blocks indefinitely (use with caution)
 * timeout_ms = 0: non-blocking try (returns RO_EAGAIN if empty/full)
 * ========================================================================= */

#ifndef ROBOTOS_RO_QUEUE_H
#define ROBOTOS_RO_QUEUE_H

#include <robotos/ro_status.h>
#include <stdint.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Special timeout value ----------------------------------------------- */

#define RO_QUEUE_WAIT_FOREVER  UINT32_MAX

/* ---- Queue handle -------------------------------------------------------- */

/**
 * ro_queue_t — Fixed-size message queue.
 *
 * Internal layout:
 *   [buffer]     → caller-provided static array
 *   [head, tail] → ring buffer indices (wrap mod capacity)
 *   [count]      → current number of items
 *   [_impl]      → platform-specific sync primitives (k_msgq / pthread_mutex)
 */
typedef struct {
    uint8_t*  buffer;       /* Caller-provided backing memory        */
    uint32_t  item_size;    /* Size of each element in bytes         */
    uint32_t  capacity;     /* Maximum number of elements            */
    volatile uint32_t head; /* Next write position (producer)        */
    volatile uint32_t tail; /* Next read position (consumer)         */
    volatile uint32_t count;/* Current fill level                    */
    uint8_t   _impl[64];   /* Backend-specific sync state (opaque)  */
} ro_queue_t;

/* ---- Lifecycle ----------------------------------------------------------- */

/**
 * Initialize a queue with caller-provided backing buffer.
 *
 * @param q         Queue object to initialize
 * @param item_size Size of each item in bytes (must be > 0)
 * @param capacity  Maximum number of items (must be > 0)
 * @param buf       Caller-owned static buffer of size >= item_size * capacity
 * @param buf_size  Total size of `buf` in bytes (validated >= item_size * capacity)
 *
 * Returns: RO_OK, RO_EINVAL if any parameter is invalid.
 */
ro_status_t ro_queue_create(ro_queue_t* q,
                            uint32_t    item_size,
                            uint32_t    capacity,
                            void*       buf,
                            uint32_t    buf_size);

/** Reset queue to empty state (drops all pending items). */
void ro_queue_reset(ro_queue_t* q);

/* ---- Thread-safe send/recv (blocking with timeout) ----------------------- */

/**
 * Enqueue an item (copy `item_size` bytes from `item`).
 *
 * @param timeout_ms  0 = non-blocking (RO_EAGAIN if full),
 *                    RO_QUEUE_WAIT_FOREVER = block indefinitely.
 * Returns: RO_OK, RO_EAGAIN (full + non-blocking), RO_ETIMEDOUT.
 */
ro_status_t ro_queue_send(ro_queue_t* q, const void* item, uint32_t timeout_ms);

/**
 * Dequeue an item (copy `item_size` bytes into `item_out`).
 *
 * @param timeout_ms  0 = non-blocking (RO_EAGAIN if empty).
 * Returns: RO_OK, RO_EAGAIN (empty + non-blocking), RO_ETIMEDOUT.
 */
ro_status_t ro_queue_recv(ro_queue_t* q, void* item_out, uint32_t timeout_ms);

/* ---- ISR-safe send/recv (non-blocking, interrupt context) ---------------- */

/**
 * Enqueue from ISR context.  Non-blocking: returns RO_EAGAIN if full.
 * MUST NOT be called from thread context.
 */
ro_status_t ro_queue_send_isr(ro_queue_t* q, const void* item);

/**
 * Dequeue from ISR context.  Non-blocking: returns RO_EAGAIN if empty.
 */
ro_status_t ro_queue_recv_isr(ro_queue_t* q, void* item_out);

/* ---- Inspection ---------------------------------------------------------- */

/** Return current number of items in the queue (non-blocking). */
uint32_t ro_queue_count(const ro_queue_t* q);

/** Return true if queue is full. */
static inline bool ro_queue_is_full(const ro_queue_t* q) {
    return q->count >= q->capacity;
}

/** Return true if queue is empty. */
static inline bool ro_queue_is_empty(const ro_queue_t* q) {
    return q->count == 0;
}

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_QUEUE_H */
