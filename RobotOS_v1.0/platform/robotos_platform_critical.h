/*
 * robotos_platform_critical.h
 * RobotOS Platform Critical Section / ISR Lock Boundary — Phase 5D.
 *
 * Provides a minimal, portable critical-section API. The implementation is
 * backend-specific: the Zephyr backend uses irq_lock/irq_unlock; the host
 * test backend uses a counter stub.
 *
 * API shape:
 *   token = robotos_platform_critical_enter();
 *   ... protected work ...
 *   robotos_platform_critical_exit(token);
 *
 * TOKEN SEMANTICS
 *   enter() returns an opaque token that must be passed to exit().
 *   The token carries backend-specific state (e.g. IRQ key).
 *   Mismatched enter/exit pairs are not detected by this API.
 *
 * LIMITATIONS (Phase 5D)
 *   - For short critical sections only (backend may disable interrupts).
 *   - ISR-safe posting from application context is NOT guaranteed at
 *     RobotOS level in Phase 5D.
 *   - This API is NOT wired into core event queue/post_event/tick yet.
 *   - Core event APIs remain single-threaded and non-ISR-safe.
 *   - No mutex or thread abstraction; no scheduler concurrency.
 *   - Interrupt latency risk if critical sections are held too long.
 *   - No public Zephyr or board types here.
 *
 * No Zephyr includes. No board types. C99 compatible.
 */

#ifndef ROBOTOS_PLATFORM_CRITICAL_H
#define ROBOTOS_PLATFORM_CRITICAL_H

#include <stdint.h>

/*
 * Opaque critical-section token.
 * uintptr_t is wide enough to hold a pointer or a 32-bit IRQ key.
 * Callers must not inspect or modify the opaque field.
 */
typedef struct {
	uintptr_t opaque;
} robotos_platform_critical_token_t;

/*
 * Enter a critical section.
 * Backend disables interrupts (Zephyr) or increments a counter (host stub).
 * Returns a token that MUST be passed to robotos_platform_critical_exit().
 * Must not be called from within a critical section on backends where nesting
 * is not supported (see backend documentation).
 */
robotos_platform_critical_token_t robotos_platform_critical_enter(void);

/*
 * Exit the critical section identified by token.
 * Token must be the value returned by the matching enter() call.
 * Restores the interrupt/lock state captured at enter time.
 */
void robotos_platform_critical_exit(robotos_platform_critical_token_t token);

#endif /* ROBOTOS_PLATFORM_CRITICAL_H */
