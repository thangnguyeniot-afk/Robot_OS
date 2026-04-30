/* ============================================================================
 * ro_status.h — RobotOS Unified Error Model
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Every Adapter and Framework operation returns ro_status_t.
 * Two return patterns exist (NEVER mix them):
 *   1. Alloc/acquire: returns non-NULL pointer on success, NULL on failure
 *   2. Operation:     returns RO_OK (0) or negative ro_status_t
 * ========================================================================= */

#ifndef ROBOTOS_RO_STATUS_H
#define ROBOTOS_RO_STATUS_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Type ---------------------------------------------------------------- */

typedef int32_t ro_status_t;

/* ---- Operational errors (recoverable) ------------------------------------ */

#define RO_OK           ((ro_status_t)  0)
#define RO_ETIMEDOUT    ((ro_status_t) -1)   /* Operation timed out             */
#define RO_EAGAIN       ((ro_status_t) -2)   /* Resource temporarily unavailable*/
#define RO_EBUSY        ((ro_status_t) -3)   /* Resource busy                   */
#define RO_ECANCELED    ((ro_status_t) -4)   /* Operation cancelled             */

/* ---- Programming errors -------------------------------------------------- */

#define RO_EINVAL       ((ro_status_t)-10)   /* Invalid argument                */
#define RO_ENOMEM       ((ro_status_t)-11)   /* Out of memory (pool exhausted)  */
#define RO_ENODEV       ((ro_status_t)-12)   /* Device not found / DT mismatch  */
#define RO_ENOTSUP      ((ro_status_t)-13)   /* Feature not supported           */
#define RO_EOVERFLOW    ((ro_status_t)-14)   /* Buffer overflow                 */

/* ---- Fatal / boot errors ------------------------------------------------- */

#define RO_EFAIL        ((ro_status_t)-20)   /* Unrecoverable failure           */
#define RO_ENOTREADY    ((ro_status_t)-21)   /* Module not initialized yet      */
#define RO_EHWINIT      ((ro_status_t)-22)   /* Hardware init failed            */
#define RO_EBOOTSEQ     ((ro_status_t)-23)   /* Boot sequence error             */

/* ---- Classification macros ----------------------------------------------- */

#define RO_SUCCEEDED(s)  ((s) == RO_OK)
#define RO_FAILED(s)     ((s) != RO_OK)
#define RO_IS_FATAL(s)   ((s) <= RO_EFAIL)

/* ---- Propagation helper -------------------------------------------------- */

/**
 * RO_TRY(expr) — evaluate `expr`, return immediately if not RO_OK.
 *
 * Usage:
 *   ro_status_t s = some_function();
 *   RO_TRY(s);
 *
 * Technique: "Early return on error" — the caller's return type must be
 * ro_status_t.  This pattern avoids deep nesting and makes the happy path
 * read linearly from top to bottom.
 */
#define RO_TRY(expr)                              \
    do {                                           \
        ro_status_t _ro_try_s = (expr);            \
        if (_ro_try_s != RO_OK) return _ro_try_s;  \
    } while (0)

/* ---- Adapter-internal translation (prototype only) ----------------------- */

/**
 * Translate a Zephyr errno (negative int) to ro_status_t.
 * Defined in src/adapter/zephyr/ro_status.c only.
 * Application code MUST NOT call this — it is adapter-internal.
 */
ro_status_t ro_from_zephyr(int zephyr_errno);

/**
 * Return a human-readable constant name for a status code.
 * e.g. ro_status_name(RO_EAGAIN) → "RO_EAGAIN"
 */
const char* ro_status_name(ro_status_t s);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_STATUS_H */
