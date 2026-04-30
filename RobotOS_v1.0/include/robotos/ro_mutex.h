/* ============================================================================
 * ro_mutex.h — RobotOS Mutex with Priority Inheritance
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming technique: Priority inheritance mutex.
 * When a high-priority thread blocks on a mutex held by a lower-priority
 * thread, the holder's priority is temporarily boosted to prevent
 * priority inversion.  This is critical for real-time safety.
 *
 * On Zephyr: wraps k_mutex (built-in priority inheritance).
 * On host:   wraps pthread_mutex_t (PTHREAD_PRIO_INHERIT if supported).
 * ========================================================================= */

#ifndef ROBOTOS_RO_MUTEX_H
#define ROBOTOS_RO_MUTEX_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Mutex handle -------------------------------------------------------- */

typedef struct {
    uint8_t _impl[64];  /* Backend-specific state (opaque) */
} ro_mutex_t;

/* ---- Lifecycle ----------------------------------------------------------- */

/**
 * Create (initialize) a mutex.
 * Returns: RO_OK, RO_ENOMEM if internal resources exhausted.
 */
ro_status_t ro_mutex_create(ro_mutex_t* mtx);

/** Destroy a mutex. MUST NOT be locked when destroyed. */
void ro_mutex_destroy(ro_mutex_t* mtx);

/* ---- Lock / Unlock ------------------------------------------------------- */

/**
 * Acquire the mutex.
 * @param timeout_ms  0 = try-lock (returns RO_EBUSY if held),
 *                    UINT32_MAX = block forever.
 * Returns: RO_OK, RO_EBUSY (try-lock failed), RO_ETIMEDOUT.
 */
ro_status_t ro_mutex_lock(ro_mutex_t* mtx, uint32_t timeout_ms);

/** Release the mutex. Caller MUST be the current owner. */
ro_status_t ro_mutex_unlock(ro_mutex_t* mtx);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_MUTEX_H */
