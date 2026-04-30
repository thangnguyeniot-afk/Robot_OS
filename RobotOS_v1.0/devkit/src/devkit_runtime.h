/*
 * devkit_runtime.h
 * Boot banner, periodic tick log, and status LED orchestration.
 * Contains the tick interval constant used by the runtime loop.
 */

#ifndef DEVKIT_RUNTIME_H
#define DEVKIT_RUNTIME_H

#define DEVKIT_TICK_MS 500

int devkit_runtime_init(void);
void devkit_runtime_run(void);

#endif /* DEVKIT_RUNTIME_H */
