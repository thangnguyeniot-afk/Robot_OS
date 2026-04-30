/* ============================================================================
 * ro_trace.h — RobotOS Trace Event Interface
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming technique: Compile-time gated trace points.
 * When CONFIG_TRACING=y (Zephyr) or ROBOTOS_TRACE_ENABLED is defined,
 * RO_TRACE_EVENT expands to an actual trace call.  Otherwise, it compiles
 * to nothing — zero overhead in release builds.
 *
 * On Zephyr: hooks into sys_trace CTF backend.
 * On host:   writes timestamped events to a trace file or stderr.
 * ========================================================================= */

#ifndef ROBOTOS_RO_TRACE_H
#define ROBOTOS_RO_TRACE_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/** Initialize the trace subsystem. Called during SYS_INIT (priority 60). */
void ro_trace_init(void);

/**
 * Record a trace event.
 * @param module   Module name (e.g. "STEPPER", "SM")
 * @param event    Event name (e.g. "move_start", "state_change")
 * @param value    Numeric payload (event-specific meaning)
 */
void ro_trace_event(const char* module, const char* event, uint32_t value);

/* ---- Compile-time gated macro -------------------------------------------- */

#if defined(CONFIG_TRACING) || defined(ROBOTOS_TRACE_ENABLED)
    #define RO_TRACE_EVENT(mod, evt, val) ro_trace_event(mod, evt, val)
#else
    #define RO_TRACE_EVENT(mod, evt, val) ((void)0)
#endif

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_TRACE_H */
