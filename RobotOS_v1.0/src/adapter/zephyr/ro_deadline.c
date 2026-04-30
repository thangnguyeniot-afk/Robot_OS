/* ============================================================================
 * ro_deadline.c — Deadline Monitoring Implementation
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 *
 * Data structure: Static table of deadline slots.
 *   - RO_DEADLINE_MAX_IDS (16) slots, each with:
 *     { id, name, budget_us, _start_us, miss_count, registered }
 *   - register() claims a slot by searching for first unregistered entry
 *   - begin() stores ro_time_us() in the slot's _start_us
 *   - end() computes elapsed = now - _start_us, increments miss_count if > budget
 *   - All O(1) after a linear search by ID (max 16 slots — effectively O(1))
 * ========================================================================= */

#include <robotos/ro_deadline.h>
#include <robotos/ro_time.h>

#include <string.h>
#include <stdbool.h>

/* ---- Slot ---------------------------------------------------------------- */

typedef struct {
    bool        registered;
    uint16_t    id;
    const char* name;
    uint32_t    budget_us;
    uint64_t    _start_us;
    volatile uint32_t miss_count;
} deadline_slot_t;

static deadline_slot_t s_table[RO_DEADLINE_MAX_IDS];

/* ---- Internal: find slot by ID ------------------------------------------- */

static deadline_slot_t* find_slot(uint16_t id)
{
    for (int i = 0; i < RO_DEADLINE_MAX_IDS; i++) {
        if (s_table[i].registered && s_table[i].id == id) {
            return &s_table[i];
        }
    }
    return NULL;
}

/* ---- API ----------------------------------------------------------------- */

ro_status_t ro_deadline_register(uint16_t id, const char* name, uint32_t budget_us)
{
    /* Check for duplicate */
    if (find_slot(id) != NULL) return RO_EINVAL;

    /* Find empty slot */
    for (int i = 0; i < RO_DEADLINE_MAX_IDS; i++) {
        if (!s_table[i].registered) {
            s_table[i].registered  = true;
            s_table[i].id          = id;
            s_table[i].name        = name;
            s_table[i].budget_us   = budget_us;
            s_table[i]._start_us   = 0;
            s_table[i].miss_count  = 0;
            return RO_OK;
        }
    }
    return RO_ENOMEM;  /* Table full */
}

void ro_deadline_begin(uint16_t id)
{
    deadline_slot_t* slot = find_slot(id);
    if (slot) {
        slot->_start_us = ro_time_us();
    }
}

void ro_deadline_end(uint16_t id)
{
    deadline_slot_t* slot = find_slot(id);
    if (slot && slot->_start_us > 0) {
        uint64_t elapsed = ro_time_us() - slot->_start_us;
        if (elapsed > slot->budget_us) {
            slot->miss_count++;
        }
        slot->_start_us = 0;
    }
}

uint32_t ro_deadline_miss_count(uint16_t id)
{
    deadline_slot_t* slot = find_slot(id);
    return slot ? slot->miss_count : 0;
}

void ro_deadline_miss_reset(uint16_t id)
{
    deadline_slot_t* slot = find_slot(id);
    if (slot) {
        slot->miss_count = 0;
    }
}

const char* ro_deadline_get_name(uint16_t id)
{
    deadline_slot_t* slot = find_slot(id);
    return slot ? slot->name : NULL;
}

uint32_t ro_deadline_get_budget(uint16_t id)
{
    deadline_slot_t* slot = find_slot(id);
    return slot ? slot->budget_us : 0;
}
