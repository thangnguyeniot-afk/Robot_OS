/* ============================================================================
 * ro_log.h — RobotOS Logging Subsystem
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming technique: ISR-safe ring buffer logger.
 *
 * Data structure: Lock-free ring buffer (single-producer variant).
 *   - Fixed-size entries: [timestamp_us : 8B][level : 1B][module : 7B][msg : 48B]
 *   - Producer appends via atomic head increment
 *   - Consumer (background flush thread) chases tail
 *   - If ring is full, entry is dropped and ro_log_drop_count incremented
 *
 * This ensures that logging in ISR context never blocks or allocates.
 *
 * On Zephyr: ring buffer backed by CONFIG_RO_LOG_RING_SIZE bytes.
 * On host:   direct fprintf to stderr for simplicity.
 * ========================================================================= */

#ifndef ROBOTOS_RO_LOG_H
#define ROBOTOS_RO_LOG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Log levels ---------------------------------------------------------- */

typedef enum {
    RO_LOG_DEBUG = 0,
    RO_LOG_INFO  = 1,
    RO_LOG_WARN  = 2,
    RO_LOG_ERROR = 3,
} ro_log_level_t;

/* ---- Core log function --------------------------------------------------- */

/**
 * Log a message.  ISR-safe: appends to ring buffer, never blocks.
 *
 * @param level   Severity level
 * @param module  Module name (max 7 chars, e.g. "STEPPER", "PID", "APP")
 * @param fmt     printf-style format string
 * @param ...     Format arguments
 */
void ro_log(ro_log_level_t level, const char* module, const char* fmt, ...)
    __attribute__((format(printf, 3, 4)));

/* ---- Convenience macros -------------------------------------------------- */

#define RO_LOG_DEBUG(mod, fmt, ...) ro_log(RO_LOG_DEBUG, mod, fmt, ##__VA_ARGS__)
#define RO_LOG_INFO(mod, fmt, ...)  ro_log(RO_LOG_INFO,  mod, fmt, ##__VA_ARGS__)
#define RO_LOG_WARN(mod, fmt, ...)  ro_log(RO_LOG_WARN,  mod, fmt, ##__VA_ARGS__)
#define RO_LOG_ERROR(mod, fmt, ...) ro_log(RO_LOG_ERROR, mod, fmt, ##__VA_ARGS__)

/* ---- Drop counter (ISR-safe read) ---------------------------------------- */

/**
 * Number of log entries dropped due to full ring buffer.
 * Reset on read via ro_log_drop_count_reset().
 */
extern volatile uint32_t ro_log_drop_count;
void ro_log_drop_count_reset(void);

/* ---- Init / Flush -------------------------------------------------------- */

/** Initialize the log subsystem. Called during SYS_INIT (priority 61). */
void ro_log_init(void);

/** Flush pending ring entries to output. Called from background thread. */
void ro_log_flush(void);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_LOG_H */
