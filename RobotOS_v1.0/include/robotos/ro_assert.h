/* ============================================================================
 * ro_assert.h — RobotOS Assertion Macro
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming technique: Compile-time and run-time assertion with a mandatory
 * message string.  Unlike bare `assert()`, RO_ASSERT always includes a
 * human-readable reason that appears in logs / core dumps.
 *
 * In release builds (NDEBUG defined), the condition is still evaluated but
 * the failure path calls a weak `ro_assert_fail()` that can be overridden
 * by the application (e.g. to blink an LED or enter safe state).
 * ========================================================================= */

#ifndef ROBOTOS_RO_ASSERT_H
#define ROBOTOS_RO_ASSERT_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Failure handler (weak symbol — override in app if needed) ----------- */

/**
 * Called when RO_ASSERT fails.  Default implementation:
 *   - Zephyr build: calls k_panic()
 *   - Host build:   calls abort()
 *
 * Parameters are provided for logging; the function MUST NOT return.
 */
__attribute__((noreturn, weak))
void ro_assert_fail(const char* file, int line, const char* msg);

/* ---- Assertion macro ----------------------------------------------------- */

/**
 * RO_ASSERT(condition, message)
 *
 * Always evaluates `condition`.  On failure, logs file:line + message
 * and calls ro_assert_fail() which does not return.
 *
 * Example:
 *   stepper_t* st = stepper_get("stepper_x");
 *   RO_ASSERT(st != NULL, "stepper_x not found in DT pool");
 */
#define RO_ASSERT(cond, msg)                                        \
    do {                                                             \
        if (!(cond)) {                                               \
            ro_assert_fail(__FILE__, __LINE__, (msg));                \
            /* noreturn — but compiler may not know; unreachable */   \
            for (;;) {}                                              \
        }                                                            \
    } while (0)

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_ASSERT_H */
