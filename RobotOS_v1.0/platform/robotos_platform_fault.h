/*
 * robotos_platform_fault.h
 * RobotOS platform fault/assert interface — Phase 5C boundary.
 *
 * Provides severity-graded fault reporting and a non-panicking assert
 * for use by devkit-level code and future portable modules.
 *
 * Rules:
 *   - Core must NOT call this API. Core errors are returned as status codes.
 *   - devkit-level code may call fault_report and ROBOTOS_PLATFORM_ASSERT.
 *   - This header contains no Zephyr types, no board types, no k_* types.
 *   - Backend selected at build time by compiling the appropriate source.
 *
 * Phase 5C scope: severity-graded fault_report and non-panicking assert only.
 * No panic by default. ROBOTOS_PLATFORM_FAULT_PANIC_ON_FATAL (compile switch,
 * default OFF) enables k_panic() in the Zephyr backend for FATAL severity only.
 *
 * Assert behavior:
 *   ROBOTOS_PLATFORM_ASSERT(cond, module, msg):
 *     - cond true:  returns true, no side effects.
 *     - cond false: calls fault_report(ERROR, module, msg), returns false.
 *     - Does NOT panic regardless of PANIC_ON_FATAL setting.
 */

#ifndef ROBOTOS_PLATFORM_FAULT_H
#define ROBOTOS_PLATFORM_FAULT_H

#include <stdbool.h>

/*
 * Fault severity levels. Higher value = higher severity.
 */
typedef enum {
    ROBOTOS_FAULT_INFO    = 0, /* informational, non-critical             */
    ROBOTOS_FAULT_WARNING = 1, /* degraded state, recoverable             */
    ROBOTOS_FAULT_ERROR   = 2, /* operation failed, serious               */
    ROBOTOS_FAULT_FATAL   = 3, /* unrecoverable; may trigger panic if opt */
} robotos_fault_severity_t;

/*
 * Report a fault with severity, module tag, and message.
 *
 * Thread-safety is backend-specific. Do not call from ISR in Phase 5C.
 * If severity == FATAL and ROBOTOS_PLATFORM_FAULT_PANIC_ON_FATAL is defined,
 * the Zephyr backend will call k_panic() after logging. Default is no panic.
 */
void robotos_platform_fault_report(robotos_fault_severity_t severity,
                                   const char *module,
                                   const char *message);

/*
 * Assert that condition is true.
 *
 * If condition is false: calls fault_report(ERROR, module, message),
 * returns false. Does NOT panic regardless of PANIC_ON_FATAL setting.
 * If condition is true: returns true with no side effects.
 *
 * Use via the ROBOTOS_PLATFORM_ASSERT macro.
 */
bool robotos_platform_assert(bool condition,
                              const char *module,
                              const char *message);

/*
 * Convenience macro. Evaluates condition exactly once.
 */
#define ROBOTOS_PLATFORM_ASSERT(cond, module, msg) \
    robotos_platform_assert((bool)(cond), (module), (msg))

#endif /* ROBOTOS_PLATFORM_FAULT_H */
