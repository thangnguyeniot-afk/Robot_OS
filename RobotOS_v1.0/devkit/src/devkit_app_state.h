/*
 * devkit_app_state.h
 * RobotOS devkit minimal application state machine (Phase 9C).
 *
 * Composes two real event sources (Phase 9A button USER+2 and
 * Phase 9B UART USER+3) into a tiny three-state machine. The first proof
 * that RobotOS-routed events from distinct hardware classes can drive
 * application-level behavior without any core/platform change.
 *
 * State machine:
 *
 *   +------+   button   +-------+   button   +--------+   button   +------+
 *   | IDLE |----------->| ARMED |----------->| ACTIVE |----------->| IDLE |
 *   +------+            +-------+            +--------+            +------+
 *      ^                                                              |
 *      |  uart 'r'/'R'                                                |
 *      +--------------------------------------------------------------+
 *
 *   uart 'a'/'A' -> ARMED   (from any state)
 *   uart 's'/'S' -> ACTIVE  (from any state)
 *   uart 'r'/'R' -> IDLE    (from any state)
 *   uart '?'     -> report current state only, no transition
 *   any other    -> ignored (incremented in ignored counter)
 *
 * Architectural shape (Phase 9C):
 *   - DEVKIT-LOCAL ONLY. No core/platform/Zephyr surface added.
 *   - Module is reentrancy-free: all entry points are called from the core
 *     dispatcher thread (button and UART handlers) or the devkit runtime
 *     thread (init / periodic log). Single CPU, single-threaded; no locks.
 *   - No new event types; no new core API; no admission/scheduler change.
 *   - No timers, dynamic allocation, or floating point.
 *   - Handler ownership is unchanged: button/UART producer handlers stay
 *     in their respective modules and *call into* this app module after
 *     their existing per-event work.
 */

#ifndef DEVKIT_APP_STATE_H
#define DEVKIT_APP_STATE_H

#include <stdint.h>

typedef enum {
	DEVKIT_APP_STATE_IDLE   = 0,
	DEVKIT_APP_STATE_ARMED  = 1,
	DEVKIT_APP_STATE_ACTIVE = 2,
} devkit_app_state_t;

typedef enum {
	DEVKIT_APP_SRC_NONE = 0,
	DEVKIT_APP_SRC_BTN  = 1,
	DEVKIT_APP_SRC_UART = 2,
} devkit_app_src_t;

typedef struct {
	devkit_app_state_t state;
	uint32_t           transitions;    /* total state changes */
	uint32_t           button_count;   /* button semantic events seen */
	uint32_t           uart_count;     /* UART bytes seen by app */
	uint32_t           ignored_count;  /* UART bytes that did not match any command */
	devkit_app_src_t   last_src;       /* source of the most recent input */
	uint8_t            last_byte;      /* last UART byte (low 8 bits); 0 if none */
} devkit_app_state_snapshot_t;

/*
 * Reset state, counters, and last-input fields. Must be called once at
 * runtime init, before any producer feeds events to this module.
 */
void devkit_app_state_init(void);

/*
 * Drive the state machine from a Phase 9A button event (USER+2).
 *
 * Cycles IDLE -> ARMED -> ACTIVE -> IDLE on every accepted-and-dispatched
 * button event. The seq argument is the button producer's per-event sequence;
 * stored only for the per-transition diagnostic log.
 *
 * Thread context only (must be called from devkit_button_handler).
 */
void devkit_app_state_on_button(uint32_t seq);

/*
 * Drive the state machine from a Phase 9B UART byte event (USER+3).
 *
 * Recognized commands (case-insensitive ASCII):
 *   'a' / 'A' -> ARMED
 *   's' / 'S' -> ACTIVE
 *   'r' / 'R' -> IDLE
 *   '?'       -> report current state (no transition)
 *   any other -> ignored (counter incremented; no transition)
 *
 * Thread context only (must be called from devkit_uart_handler).
 */
void devkit_app_state_on_uart_byte(uint8_t byte, uint32_t handler_count);

/*
 * Snapshot the current state and counters into a caller-owned struct.
 * If out is NULL the call is a no-op.
 */
void devkit_app_state_get_snapshot(devkit_app_state_snapshot_t *out);

/*
 * Return the canonical name string for a state enum value.
 * Returns "IDLE", "ARMED", "ACTIVE", or "UNKNOWN" for out-of-range values.
 * Result is a string literal (read-only); safe to call from thread context.
 * Added Phase 9E so devkit_uart_producer can format UART TX responses without
 * duplicating the name-lookup logic.
 */
const char *devkit_app_state_state_name(devkit_app_state_t state);

/*
 * Emit one ROBOTOS_APP log line via Zephyr LOG_INF.
 *
 * Stable single-line format (state and source names only; integers/hex):
 *   ROBOTOS_APP state=NAME transitions=N button=N uart=N ignored=N \
 *               last_src=NAME last_byte=0xNN
 *
 * Where NAME for state ∈ {IDLE, ARMED, ACTIVE} and last_src ∈ {NONE, BTN, UART}.
 *
 * Thread context only (Zephyr LOG_INF deferred path; not safe in ISR).
 */
void devkit_app_state_log_snapshot(void);

#endif /* DEVKIT_APP_STATE_H */
