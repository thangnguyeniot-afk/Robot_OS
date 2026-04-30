/* ============================================================================
 * ro_trace.c — Zephyr Trace Backend
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 * ========================================================================= */

#include <robotos/ro_trace.h>
#include <robotos/ro_time.h>
#include <stdio.h>

void ro_trace_init(void)
{
    /* On Zephyr: sys_trace_init() is called by the kernel.
     * On host: no-op — trace events go to stderr. */
}

void ro_trace_event(const char* module, const char* event, uint32_t value)
{
#if defined(CONFIG_TRACING) || defined(ROBOTOS_TRACE_ENABLED)
    /* Minimal implementation: print to console.
     * Production: hook into CTF or SystemView backend. */
    printf("[TRACE %10llu] %s.%s = %u\n",
           (unsigned long long)ro_time_us(), module, event, value);
#else
    (void)module; (void)event; (void)value;
#endif
}
