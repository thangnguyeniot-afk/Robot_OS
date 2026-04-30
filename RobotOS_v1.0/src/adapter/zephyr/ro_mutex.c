/* ============================================================================
 * ro_mutex.c — Zephyr Mutex Backend (Priority Inheritance)
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 * ========================================================================= */

#include <robotos/ro_mutex.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/kernel.h>
#endif

/* ---- Internal: access k_mutex stored inside _impl[] ---------------------- */

#ifndef ROBOTOS_HOST_BUILD
static inline struct k_mutex* get_kmutex(ro_mutex_t* mtx)
{
    return (struct k_mutex*)mtx->_impl;
}
#endif

/* ---- API ----------------------------------------------------------------- */

ro_status_t ro_mutex_create(ro_mutex_t* mtx)
{
    RO_ASSERT(mtx != NULL, "ro_mutex_create: NULL mutex");

#ifndef ROBOTOS_HOST_BUILD
    k_mutex_init(get_kmutex(mtx));
#endif

    return RO_OK;
}

void ro_mutex_destroy(ro_mutex_t* mtx)
{
    (void)mtx;  /* Zephyr k_mutex has no explicit destroy */
}

ro_status_t ro_mutex_lock(ro_mutex_t* mtx, uint32_t timeout_ms)
{
    RO_ASSERT(mtx != NULL, "ro_mutex_lock: NULL mutex");

#ifndef ROBOTOS_HOST_BUILD
    k_timeout_t timeout;
    if (timeout_ms == UINT32_MAX) {
        timeout = K_FOREVER;
    } else if (timeout_ms == 0) {
        timeout = K_NO_WAIT;
    } else {
        timeout = K_MSEC(timeout_ms);
    }

    int ret = k_mutex_lock(get_kmutex(mtx), timeout);
    if (ret == 0) return RO_OK;
    return (timeout_ms == 0) ? RO_EBUSY : RO_ETIMEDOUT;
#else
    return RO_OK;
#endif
}

ro_status_t ro_mutex_unlock(ro_mutex_t* mtx)
{
    RO_ASSERT(mtx != NULL, "ro_mutex_unlock: NULL mutex");

#ifndef ROBOTOS_HOST_BUILD
    k_mutex_unlock(get_kmutex(mtx));
#endif

    return RO_OK;
}
