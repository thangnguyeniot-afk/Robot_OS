/*
 * robotos_platform_log.h
 * RobotOS platform logging interface — Phase 5A boundary seed.
 *
 * This header defines the minimal, backend-agnostic logging API that
 * RobotOS core and other portable modules use for diagnostic output.
 *
 * Rules:
 *   - Core may include this header.
 *   - Core must NOT include <zephyr/logging/log.h> directly.
 *   - The backend (Zephyr, host stub, or future custom) is selected at
 *     build time by compiling the appropriate implementation source.
 *   - This header contains no Zephyr types, no board types, no k_*
 *     types, no GPIO or DTS references.
 *
 * Phase 5A scope: logging boundary only.
 * Thread/mutex/time/assert/fault abstraction is NOT in scope here.
 */

#ifndef ROBOTOS_PLATFORM_LOG_H
#define ROBOTOS_PLATFORM_LOG_H

#include <stdarg.h>

/*
 * Log severity levels, ordered from least to most severe.
 * Backends map these to their native severity concept.
 */
typedef enum {
    ROBOTOS_LOG_LEVEL_DEBUG = 0,
    ROBOTOS_LOG_LEVEL_INFO,
    ROBOTOS_LOG_LEVEL_WARN,
    ROBOTOS_LOG_LEVEL_ERROR,
} robotos_log_level_t;

/*
 * Emit a formatted log message through the platform backend.
 *
 * level   — severity of the message.
 * module  — short string identifying the calling module (e.g. "robotos_core").
 * fmt     — printf-style format string.
 * ...     — format arguments.
 *
 * Thread-safety is backend-specific. In Phase 5A, single-threaded use only.
 * Do not call from ISR context in this phase.
 */
void robotos_platform_logf(robotos_log_level_t level,
                           const char         *module,
                           const char         *fmt, ...);

#endif /* ROBOTOS_PLATFORM_LOG_H */
