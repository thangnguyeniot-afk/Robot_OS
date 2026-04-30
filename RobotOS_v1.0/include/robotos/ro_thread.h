/* ============================================================================
 * ro_thread.h — RobotOS Thread Abstraction
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Programming techniques:
 *   - Opaque priority type: ro_priority_t is a struct wrapping int16_t,
 *     preventing accidental construction from bare integers.  Only the
 *     six named presets (RO_PRIO_RT_PULSE, etc.) are valid.
 *   - Static stack allocation: RO_THREAD_STACK_DEFINE places the stack
 *     in a linker section on Zephyr, or as a plain static array on host.
 *   - Thread slot pool: Adapter maintains CONFIG_RO_MAX_THREADS static
 *     slots.  ro_thread_create() claims a slot; ro_thread_destroy() frees.
 * ========================================================================= */

#ifndef ROBOTOS_RO_THREAD_H
#define ROBOTOS_RO_THREAD_H

#include <robotos/ro_status.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Priority type (opaque struct — cannot construct from bare int) ------ */

/**
 * ro_priority_t — Thread scheduling priority.
 *
 * Lower .v = higher priority.  Negative values = cooperative (non-preemptible),
 * zero/positive = preemptive.
 *
 * Usage: pass one of the six named presets.  Direct construction like
 * (ro_priority_t){.v = 42} is technically possible but violates the
 * naming contract and will be flagged by CI `ro_check_thread_roster`.
 */
typedef struct {
    int16_t v;
} ro_priority_t;

/* ---- Named priority presets ---------------------------------------------- */

/** ISR-adjacent pulse generation (cooperative, highest app priority) */
#define RO_PRIO_RT_PULSE     ((ro_priority_t){ .v = -16 })

/** Real-time control loops — PID, planner tick (cooperative) */
#define RO_PRIO_RT_CONTROL   ((ro_priority_t){ .v = -10 })

/** Monitoring — watchdog, diagnostic (cooperative) */
#define RO_PRIO_RT_MONITOR   ((ro_priority_t){ .v =  -2 })

/** Framework state machine event loop (preemptive, neutral) */
#define RO_PRIO_FRAMEWORK    ((ro_priority_t){ .v =   0 })

/** Application-level tasks (preemptive) */
#define RO_PRIO_APP          ((ro_priority_t){ .v =   5 })

/** Background / housekeeping — shell, logging (preemptive, lowest) */
#define RO_PRIO_BACKGROUND   ((ro_priority_t){ .v =  12 })

/* ---- Stack allocation macro ---------------------------------------------- */

/**
 * RO_THREAD_STACK_DEFINE(name, size)
 *
 * Declares a static stack buffer.  On Zephyr, expands to K_THREAD_STACK_DEFINE
 * (places in the .noinit section with proper alignment).  On host builds,
 * it is a plain static uint8_t array.
 */
#ifdef ROBOTOS_HOST_BUILD
    #define RO_THREAD_STACK_DEFINE(name, size) \
        static uint8_t name[size]
#else
    /* In Zephyr build, adapter/zephyr/ro_thread.c provides the real macro.
     * We forward-declare the symbol here; the .c file uses K_THREAD_STACK_DEFINE. */
    #define RO_THREAD_STACK_DEFINE(name, size) \
        static uint8_t __aligned(8) name[size]
#endif

/* ---- Thread handle (opaque) ---------------------------------------------- */

typedef struct ro_thread ro_thread_t;

/* ---- Thread entry function ----------------------------------------------- */

typedef void (*ro_thread_entry_t)(void* arg);

/* ---- Thread configuration ------------------------------------------------ */

typedef struct {
    const char*       name;       /* Human-readable name (for debug/roster) */
    uint8_t*          stack;      /* Pointer to static stack buffer         */
    uint32_t          stack_size; /* Stack size in bytes                    */
    ro_priority_t     priority;   /* One of the RO_PRIO_* presets          */
    ro_thread_entry_t entry;      /* Thread entry function                 */
    void*             arg;        /* Argument passed to entry()            */
} ro_thread_config_t;

/* ---- Lifecycle API ------------------------------------------------------- */

/**
 * Create a thread from a static slot pool.
 * Returns NULL if CONFIG_RO_MAX_THREADS is exhausted.
 *
 * The thread begins executing immediately (or when the scheduler
 * next preempts, depending on priority).
 */
ro_thread_t* ro_thread_create(const ro_thread_config_t* cfg);

/**
 * Destroy a thread and return its slot to the pool.
 * The thread MUST have terminated (joined) before calling this.
 */
void ro_thread_destroy(ro_thread_t* t);

/* ---- Thread control ------------------------------------------------------ */

/** Sleep the calling thread for `ms` milliseconds (yields CPU). */
void ro_thread_sleep_ms(uint32_t ms);

/** Yield the CPU to any same-or-higher priority runnable thread. */
void ro_thread_yield(void);

/** Get the human-readable name of a thread. */
const char* ro_thread_get_name(const ro_thread_t* t);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_THREAD_H */
