/*
 * robotos_core.h
 * RobotOS portable core — Phase 4A bootstrap API.
 *
 * Phase 4A scope: lifecycle init/tick only.
 * Scheduler, event bus, and peripherals are intentionally not implemented yet.
 * No Zephyr or board-specific types exposed in this header.
 */

#ifndef ROBOTOS_CORE_H
#define ROBOTOS_CORE_H

/*
 * Return the core version string.
 * Identifies the bootstrap phase in log output.
 */
const char *robotos_core_version(void);

/*
 * Initialize the RobotOS core.
 * Must be called once during system boot before robotos_core_tick().
 * Returns 0 on success, negative on failure.
 */
int robotos_core_init(void);

/*
 * Advance the RobotOS core by one tick.
 * Called once per devkit tick interval (DEVKIT_TICK_MS).
 * Core has no internal timer — timing is owned by the caller.
 * Returns 0 on success, negative on failure.
 */
int robotos_core_tick(void);

#endif /* ROBOTOS_CORE_H */
