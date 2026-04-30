/* ro_trace.c — Host Trace Backend */
#include <robotos/ro_trace.h>
#include <robotos/ro_time.h>
#include <stdio.h>

void ro_trace_init(void) { /* no-op on host */ }

void ro_trace_event(const char* module, const char* event, uint32_t value)
{
#ifdef ROBOTOS_TRACE_ENABLED
    fprintf(stderr, "[TRACE %10llu] %s.%s = %u\n",
            (unsigned long long)ro_time_us(), module, event, value);
#else
    (void)module; (void)event; (void)value;
#endif
}
