/*
 * robotos_platform_critical_host_stub.h
 * Host-only inspection API for the platform critical-section stub.
 *
 * These helpers are NOT part of the public platform interface.
 * They exist only in the host test environment to verify critical-section
 * enter/exit semantics without hardware interrupts.
 *
 * Phase 5D — Platform Critical Section / ISR Lock Boundary.
 */

#ifndef ROBOTOS_PLATFORM_CRITICAL_HOST_STUB_H
#define ROBOTOS_PLATFORM_CRITICAL_HOST_STUB_H

#ifdef __cplusplus
extern "C" {
#endif

/* Reset all counters and depth to zero. Call before each test scenario. */
void robotos_platform_critical_host_reset(void);

/* Cumulative count of enter() calls since last reset. */
unsigned int robotos_platform_critical_host_enter_count(void);

/* Cumulative count of exit() calls since last reset. */
unsigned int robotos_platform_critical_host_exit_count(void);

/* Current nesting depth (enter count minus exit count, floored at 0). */
unsigned int robotos_platform_critical_host_current_depth(void);

/* Maximum nesting depth observed since last reset. */
unsigned int robotos_platform_critical_host_max_depth(void);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_PLATFORM_CRITICAL_HOST_STUB_H */
