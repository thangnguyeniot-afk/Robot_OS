/* ro_log.c — Host Log Backend (fprintf to stderr) */
#include <robotos/ro_log.h>
#include <robotos/ro_time.h>
#include <stdarg.h>
#include <stdio.h>

volatile uint32_t ro_log_drop_count = 0;

static const char* lvl_str(ro_log_level_t l) {
    switch(l) {
        case RO_LOG_DEBUG: return "DBG";
        case RO_LOG_INFO:  return "INF";
        case RO_LOG_WARN:  return "WRN";
        case RO_LOG_ERROR: return "ERR";
        default:           return "???";
    }
}

void ro_log(ro_log_level_t level, const char* module, const char* fmt, ...)
{
    fprintf(stderr, "[%10llu] %s %-7s ",
            (unsigned long long)ro_time_us(),
            lvl_str(level),
            module ? module : "");
    va_list args;
    va_start(args, fmt);
    vfprintf(stderr, fmt, args);
    va_end(args);
    fprintf(stderr, "\n");
}

void ro_log_init(void) { ro_log_drop_count = 0; }
void ro_log_flush(void) { fflush(stderr); }
void ro_log_drop_count_reset(void) { ro_log_drop_count = 0; }
