/* ============================================================================
 * ro_thread.c — Zephyr Thread Backend
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 *
 * Implementation technique: Static thread slot pool.
 *   - Array of CONFIG_RO_MAX_THREADS slots, each containing a k_thread + metadata
 *   - Bitmap tracks free/used slots (same pattern as ro_pool bitmap allocator)
 *   - ro_thread_create(): find free slot → k_thread_create() → return slot ptr
 *   - ro_thread_destroy(): k_thread_abort() → mark slot free
 *
 * Priority mapping:
 *   ro_priority_t.v maps directly to Zephyr thread priority:
 *     negative = cooperative (non-preemptible)
 *     zero/positive = preemptive
 * ========================================================================= */

#include <robotos/ro_thread.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/kernel.h>
#endif

#include <string.h>

/* ---- Configuration ------------------------------------------------------- */

#ifndef CONFIG_RO_MAX_THREADS
#define CONFIG_RO_MAX_THREADS 8
#endif

/* ---- Thread slot --------------------------------------------------------- */

typedef struct ro_thread {
    bool                in_use;
    const char*         name;
    ro_thread_entry_t   entry;
    void*               arg;
#ifndef ROBOTOS_HOST_BUILD
    struct k_thread     k_thread;
    k_tid_t             tid;
#endif
} ro_thread_slot_t;

static ro_thread_slot_t s_slots[CONFIG_RO_MAX_THREADS];

/* ---- Slot allocation ----------------------------------------------------- */

static ro_thread_slot_t* slot_alloc(void)
{
    for (int i = 0; i < CONFIG_RO_MAX_THREADS; i++) {
        if (!s_slots[i].in_use) {
            s_slots[i].in_use = true;
            return &s_slots[i];
        }
    }
    return NULL;
}

static void slot_free(ro_thread_slot_t* slot)
{
    memset(slot, 0, sizeof(*slot));
}

/* ---- Zephyr thread entry wrapper ---------------------------------------- */

#ifndef ROBOTOS_HOST_BUILD
static void zephyr_thread_entry(void* p1, void* p2, void* p3)
{
    (void)p2; (void)p3;
    ro_thread_slot_t* slot = (ro_thread_slot_t*)p1;
    if (slot->entry) {
        slot->entry(slot->arg);
    }
}
#endif

/* ---- Public API ---------------------------------------------------------- */

ro_thread_t* ro_thread_create(const ro_thread_config_t* cfg)
{
    RO_ASSERT(cfg != NULL, "ro_thread_create: NULL config");
    RO_ASSERT(cfg->stack != NULL, "ro_thread_create: NULL stack");
    RO_ASSERT(cfg->entry != NULL, "ro_thread_create: NULL entry");
    RO_ASSERT(cfg->stack_size > 0, "ro_thread_create: zero stack_size");

    ro_thread_slot_t* slot = slot_alloc();
    if (slot == NULL) return NULL;

    slot->name  = cfg->name ? cfg->name : "unnamed";
    slot->entry = cfg->entry;
    slot->arg   = cfg->arg;

#ifndef ROBOTOS_HOST_BUILD
    slot->tid = k_thread_create(
        &slot->k_thread,
        (k_thread_stack_t*)cfg->stack,
        cfg->stack_size,
        zephyr_thread_entry,
        slot, NULL, NULL,
        cfg->priority.v,    /* Direct mapping: ro_priority_t.v → Zephyr prio */
        0,                  /* Options */
        K_NO_WAIT           /* Start immediately */
    );
    k_thread_name_set(slot->tid, slot->name);
#endif

    return (ro_thread_t*)slot;
}

void ro_thread_destroy(ro_thread_t* t)
{
    if (t == NULL) return;
    ro_thread_slot_t* slot = (ro_thread_slot_t*)t;

#ifndef ROBOTOS_HOST_BUILD
    k_thread_abort(slot->tid);
#endif

    slot_free(slot);
}

void ro_thread_sleep_ms(uint32_t ms)
{
#ifndef ROBOTOS_HOST_BUILD
    k_msleep((int32_t)ms);
#endif
}

void ro_thread_yield(void)
{
#ifndef ROBOTOS_HOST_BUILD
    k_yield();
#endif
}

const char* ro_thread_get_name(const ro_thread_t* t)
{
    if (t == NULL) return "(null)";
    const ro_thread_slot_t* slot = (const ro_thread_slot_t*)t;
    return slot->name;
}
