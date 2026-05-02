/*
 * robotos_platform_critical_zephyr.c
 * Zephyr backend for platform critical-section boundary (Phase 5D).
 *
 * Uses irq_lock() / irq_unlock() — global IRQ lock suitable for short
 * critical sections in thread context. Nested calls are supported on
 * ARMv7-M (BASEPRI masking); the key returned by irq_lock() tracks the
 * pre-lock BASEPRI value and irq_unlock() restores it.
 *
 * LIMITATIONS
 *   - Not wired into core event queue or post_event in Phase 5D.
 *   - For short critical sections only; holding for long durations degrades
 *     interrupt latency.
 *   - ISR-safe posting is not claimed at RobotOS level in Phase 5D.
 */

#include "robotos_platform_critical.h"

#include <zephyr/kernel.h>

robotos_platform_critical_token_t robotos_platform_critical_enter(void)
{
	robotos_platform_critical_token_t token;
	unsigned int key = irq_lock();
	token.opaque = (uintptr_t)key;
	return token;
}

void robotos_platform_critical_exit(robotos_platform_critical_token_t token)
{
	unsigned int key = (unsigned int)token.opaque;
	irq_unlock(key);
}
