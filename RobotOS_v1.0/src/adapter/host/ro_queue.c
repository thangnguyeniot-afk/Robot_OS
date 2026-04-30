/* ============================================================================
 * ro_queue.c — Host Queue Backend (Manual Ring Buffer)
 * ============================================================================
 * Layer: Adapter / Host build
 *
 * Data structure: Ring buffer with mutex protection.
 *   head → next write slot, tail → next read slot, count → fill level.
 *   Items are memcpy'd in/out.  Blocking waits use Sleep + polling
 *   (acceptable for host test builds; not production-grade).
 * ========================================================================= */

#include <robotos/ro_queue.h>
#include <robotos/ro_assert.h>
#include <robotos/ro_time.h>

#include <string.h>

#ifdef _WIN32
    #include <windows.h>
    typedef CRITICAL_SECTION host_mutex_t;
    #define HOST_MUTEX_INIT(m)   InitializeCriticalSection(m)
    #define HOST_MUTEX_LOCK(m)   EnterCriticalSection(m)
    #define HOST_MUTEX_UNLOCK(m) LeaveCriticalSection(m)
#else
    #include <pthread.h>
    #include <unistd.h>
    typedef pthread_mutex_t host_mutex_t;
    #define HOST_MUTEX_INIT(m)   pthread_mutex_init(m, NULL)
    #define HOST_MUTEX_LOCK(m)   pthread_mutex_lock(m)
    #define HOST_MUTEX_UNLOCK(m) pthread_mutex_unlock(m)
#endif

/* ---- Access mutex inside _impl[] ---------------------------------------- */

static inline host_mutex_t* get_lock(ro_queue_t* q)
{
    return (host_mutex_t*)q->_impl;
}

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

    HOST_MUTEX_INIT(get_lock(q));
    return RO_OK;
}

void ro_queue_reset(ro_queue_t* q)
{
    if (q == NULL) return;
    HOST_MUTEX_LOCK(get_lock(q));
    q->head  = 0;
    q->tail  = 0;
    q->count = 0;
    HOST_MUTEX_UNLOCK(get_lock(q));
}

/* ---- Internal push/pop (caller holds lock) ------------------------------- */

static bool queue_push_locked(ro_queue_t* q, const void* item)
{
    if (q->count >= q->capacity) return false;
    memcpy(q->buffer + q->head * q->item_size, item, q->item_size);
    q->head = (q->head + 1) % q->capacity;
    q->count++;
    return true;
}

static bool queue_pop_locked(ro_queue_t* q, void* item_out)
{
    if (q->count == 0) return false;
    memcpy(item_out, q->buffer + q->tail * q->item_size, q->item_size);
    q->tail = (q->tail + 1) % q->capacity;
    q->count--;
    return true;
}

/* ---- Thread-safe send/recv ----------------------------------------------- */

ro_status_t ro_queue_send(ro_queue_t* q, const void* item, uint32_t timeout_ms)
{
    RO_ASSERT(q != NULL && item != NULL, "ro_queue_send: NULL arg");

    uint32_t start = ro_time_ms();
    for (;;) {
        HOST_MUTEX_LOCK(get_lock(q));
        bool ok = queue_push_locked(q, item);
        HOST_MUTEX_UNLOCK(get_lock(q));

        if (ok) return RO_OK;
        if (timeout_ms == 0) return RO_EAGAIN;

        uint32_t elapsed = ro_time_ms() - start;
        if (timeout_ms != RO_QUEUE_WAIT_FOREVER && elapsed >= timeout_ms) {
            return RO_ETIMEDOUT;
        }
#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
    }
}

ro_status_t ro_queue_recv(ro_queue_t* q, void* item_out, uint32_t timeout_ms)
{
    RO_ASSERT(q != NULL && item_out != NULL, "ro_queue_recv: NULL arg");

    uint32_t start = ro_time_ms();
    for (;;) {
        HOST_MUTEX_LOCK(get_lock(q));
        bool ok = queue_pop_locked(q, item_out);
        HOST_MUTEX_UNLOCK(get_lock(q));

        if (ok) return RO_OK;
        if (timeout_ms == 0) return RO_EAGAIN;

        uint32_t elapsed = ro_time_ms() - start;
        if (timeout_ms != RO_QUEUE_WAIT_FOREVER && elapsed >= timeout_ms) {
            return RO_ETIMEDOUT;
        }
#ifdef _WIN32
        Sleep(1);
#else
        usleep(1000);
#endif
    }
}

/* ---- ISR-safe (same as regular on host — single process) ----------------- */

ro_status_t ro_queue_send_isr(ro_queue_t* q, const void* item)
{
    return ro_queue_send(q, item, 0);
}

ro_status_t ro_queue_recv_isr(ro_queue_t* q, void* item_out)
{
    return ro_queue_recv(q, item_out, 0);
}

/* ---- Inspection ---------------------------------------------------------- */

uint32_t ro_queue_count(const ro_queue_t* q)
{
    return q ? q->count : 0;
}
