/* ============================================================================
 * ro_queue.c — Zephyr Queue Backend (Fixed-Size Ring Buffer)
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 *
 * Implementation technique:
 *   - Uses Zephyr k_msgq for thread synchronization (blocking send/recv)
 *   - The k_msgq itself is initialized with the caller-provided buffer
 *   - ISR-safe variants use k_msgq_put/k_msgq_get from ISR context
 *
 *   Alternative approach (used for host build): manual ring buffer with
 *   head/tail indices + condition variable. The Zephyr build delegates
 *   entirely to k_msgq which already implements a correct ring.
 * ========================================================================= */

#include <robotos/ro_queue.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/kernel.h>
#endif

#include <string.h>

/* ---- Internal: access k_msgq stored inside _impl[] ----------------------- */

#ifndef ROBOTOS_HOST_BUILD
static inline struct k_msgq* get_msgq(ro_queue_t* q)
{
    return (struct k_msgq*)q->_impl;
}
#endif

/* ---- Lifecycle ----------------------------------------------------------- */

ro_status_t ro_queue_create(ro_queue_t* q,
                            uint32_t    item_size,
                            uint32_t    capacity,
                            void*       buf,
                            uint32_t    buf_size)
{
    RO_ASSERT(q != NULL, "ro_queue_create: NULL queue");
    if (item_size == 0 || capacity == 0 || buf == NULL) return RO_EINVAL;
    if (buf_size < item_size * capacity) return RO_EINVAL;

    memset(q, 0, sizeof(*q));
    q->buffer    = (uint8_t*)buf;
    q->item_size = item_size;
    q->capacity  = capacity;
    q->head      = 0;
    q->tail      = 0;
    q->count     = 0;

#ifndef ROBOTOS_HOST_BUILD
    k_msgq_init(get_msgq(q), (char*)buf, item_size, capacity);
#endif

    return RO_OK;
}

void ro_queue_reset(ro_queue_t* q)
{
    if (q == NULL) return;
    q->head  = 0;
    q->tail  = 0;
    q->count = 0;

#ifndef ROBOTOS_HOST_BUILD
    k_msgq_purge(get_msgq(q));
#endif
}

/* ---- Thread-safe send/recv ----------------------------------------------- */

ro_status_t ro_queue_send(ro_queue_t* q, const void* item, uint32_t timeout_ms)
{
    RO_ASSERT(q != NULL, "ro_queue_send: NULL queue");
    RO_ASSERT(item != NULL, "ro_queue_send: NULL item");

#ifndef ROBOTOS_HOST_BUILD
    k_timeout_t timeout;
    if (timeout_ms == RO_QUEUE_WAIT_FOREVER) {
        timeout = K_FOREVER;
    } else if (timeout_ms == 0) {
        timeout = K_NO_WAIT;
    } else {
        timeout = K_MSEC(timeout_ms);
    }

    int ret = k_msgq_put(get_msgq(q), item, timeout);
    if (ret == 0) {
        q->count = k_msgq_num_used_get(get_msgq(q));
        return RO_OK;
    }
    return (ret == -EAGAIN || ret == -ENOMSG) ? RO_EAGAIN : RO_ETIMEDOUT;
#else
    (void)timeout_ms;
    return RO_ENOTSUP;
#endif
}

ro_status_t ro_queue_recv(ro_queue_t* q, void* item_out, uint32_t timeout_ms)
{
    RO_ASSERT(q != NULL, "ro_queue_recv: NULL queue");
    RO_ASSERT(item_out != NULL, "ro_queue_recv: NULL item_out");

#ifndef ROBOTOS_HOST_BUILD
    k_timeout_t timeout;
    if (timeout_ms == RO_QUEUE_WAIT_FOREVER) {
        timeout = K_FOREVER;
    } else if (timeout_ms == 0) {
        timeout = K_NO_WAIT;
    } else {
        timeout = K_MSEC(timeout_ms);
    }

    int ret = k_msgq_get(get_msgq(q), item_out, timeout);
    if (ret == 0) {
        q->count = k_msgq_num_used_get(get_msgq(q));
        return RO_OK;
    }
    return (ret == -EAGAIN || ret == -ENOMSG) ? RO_EAGAIN : RO_ETIMEDOUT;
#else
    (void)timeout_ms;
    return RO_ENOTSUP;
#endif
}

/* ---- ISR-safe send/recv -------------------------------------------------- */

ro_status_t ro_queue_send_isr(ro_queue_t* q, const void* item)
{
    RO_ASSERT(q != NULL, "ro_queue_send_isr: NULL queue");

#ifndef ROBOTOS_HOST_BUILD
    int ret = k_msgq_put(get_msgq(q), item, K_NO_WAIT);
    if (ret == 0) {
        q->count = k_msgq_num_used_get(get_msgq(q));
        return RO_OK;
    }
    return RO_EAGAIN;
#else
    (void)item;
    return RO_ENOTSUP;
#endif
}

ro_status_t ro_queue_recv_isr(ro_queue_t* q, void* item_out)
{
    RO_ASSERT(q != NULL, "ro_queue_recv_isr: NULL queue");

#ifndef ROBOTOS_HOST_BUILD
    int ret = k_msgq_get(get_msgq(q), item_out, K_NO_WAIT);
    if (ret == 0) {
        q->count = k_msgq_num_used_get(get_msgq(q));
        return RO_OK;
    }
    return RO_EAGAIN;
#else
    (void)item_out;
    return RO_ENOTSUP;
#endif
}

/* ---- Inspection ---------------------------------------------------------- */

uint32_t ro_queue_count(const ro_queue_t* q)
{
    if (q == NULL) return 0;

#ifndef ROBOTOS_HOST_BUILD
    return k_msgq_num_used_get((struct k_msgq*)q->_impl);
#else
    return q->count;
#endif
}
