/* ============================================================================
 * ro_log.c — Zephyr Logging Backend (ISR-Safe Ring Buffer)
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 *
 * Data structure: Lock-free SPSC ring buffer.
 *
 * Layout:
 *   ring[0..CONFIG_RO_LOG_RING_SIZE-1] → fixed-size log entries
 *   Each entry = 64 bytes:
 *     [ timestamp_us : 8B ]
 *     [ level        : 1B ]
 *     [ module       : 7B ]
 *     [ message      : 48B]
 *
 * Producer (ro_log):
 *   1. Atomically increment head index (mod ring_size)
 *   2. Write entry at new head position
 *   3. If ring full, increment drop count instead
 *
 * Consumer (ro_log_flush):
 *   1. Read entry at tail
 *   2. Print to console
 *   3. Advance tail
 *
 * ISR safety: producer never blocks, never allocs.
 * ========================================================================= */

#include <robotos/ro_log.h>
#include <robotos/ro_time.h>

#include <stdarg.h>
#include <stdio.h>
#include <string.h>

/* ---- Configuration ------------------------------------------------------- */

#ifndef CONFIG_RO_LOG_RING_SIZE
#define CONFIG_RO_LOG_RING_SIZE 2048
#endif

#define LOG_ENTRY_SIZE  64
#define LOG_MSG_MAX     48
#define LOG_MODULE_MAX  7
#define LOG_RING_COUNT  (CONFIG_RO_LOG_RING_SIZE / LOG_ENTRY_SIZE)

/* ---- Log entry ----------------------------------------------------------- */

typedef struct {
    uint64_t        timestamp_us;
    uint8_t         level;
    char            module[LOG_MODULE_MAX + 1];
    char            message[LOG_MSG_MAX];
} log_entry_t;

_Static_assert(sizeof(log_entry_t) <= LOG_ENTRY_SIZE, "log_entry_t exceeds 64 bytes");

/* ---- Ring buffer --------------------------------------------------------- */

static log_entry_t   s_ring[LOG_RING_COUNT];
static volatile uint32_t s_head = 0;  /* Next write position */
static volatile uint32_t s_tail = 0;  /* Next read position  */

volatile uint32_t ro_log_drop_count = 0;

/* ---- Level names --------------------------------------------------------- */

static const char* level_str(ro_log_level_t level)
{
    switch (level) {
        case RO_LOG_DEBUG: return "DBG";
        case RO_LOG_INFO:  return "INF";
        case RO_LOG_WARN:  return "WRN";
        case RO_LOG_ERROR: return "ERR";
        default:           return "???";
    }
}

/* ---- Core log function --------------------------------------------------- */

void ro_log(ro_log_level_t level, const char* module, const char* fmt, ...)
{
    /* Check if ring is full */
    uint32_t next_head = (s_head + 1) % LOG_RING_COUNT;
    if (next_head == s_tail) {
        ro_log_drop_count++;
        return;  /* Ring full — drop entry, never block */
    }

    log_entry_t* entry = &s_ring[s_head];
    entry->timestamp_us = ro_time_us();
    entry->level = (uint8_t)level;

    /* Copy module name (truncate to 7 chars) */
    if (module) {
        strncpy(entry->module, module, LOG_MODULE_MAX);
        entry->module[LOG_MODULE_MAX] = '\0';
    } else {
        entry->module[0] = '\0';
    }

    /* Format message */
    va_list args;
    va_start(args, fmt);
    vsnprintf(entry->message, LOG_MSG_MAX, fmt, args);
    va_end(args);

    s_head = next_head;
}

/* ---- Flush --------------------------------------------------------------- */

void ro_log_flush(void)
{
    while (s_tail != s_head) {
        const log_entry_t* entry = &s_ring[s_tail];
        printf("[%10llu] %s %-7s %s\n",
               (unsigned long long)entry->timestamp_us,
               level_str((ro_log_level_t)entry->level),
               entry->module,
               entry->message);
        s_tail = (s_tail + 1) % LOG_RING_COUNT;
    }
}

/* ---- Init / Reset -------------------------------------------------------- */

void ro_log_init(void)
{
    s_head = 0;
    s_tail = 0;
    ro_log_drop_count = 0;
    memset(s_ring, 0, sizeof(s_ring));
}

void ro_log_drop_count_reset(void)
{
    ro_log_drop_count = 0;
}
