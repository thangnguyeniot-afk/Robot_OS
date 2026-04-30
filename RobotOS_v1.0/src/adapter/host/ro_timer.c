/* ============================================================================
 * ro_timer.c — Host Timer Backend
 * ============================================================================
 * Layer: Adapter / Host build
 *
 * Uses ro_time_us() + Sleep for the timer-sync pattern on host.
 * ========================================================================= */

#include <robotos/ro_timer.h>
#include <robotos/ro_time.h>
#include <robotos/ro_assert.h>

#include <string.h>

#ifdef _WIN32
    #include <windows.h>
#else
    #include <unistd.h>
#endif

/* ---- Store next-deadline inside _impl[] ---------------------------------- */

typedef struct {
    uint64_t next_deadline_us;
} timer_host_data_t;

static inline timer_host_data_t* get_data(ro_timer_t* tmr) {
    return (timer_host_data_t*)tmr->_impl;
}

/* ---- API ----------------------------------------------------------------- */

ro_status_t ro_timer_init(ro_timer_t* tmr, const char* name)
{
    RO_ASSERT(tmr != NULL, "ro_timer_init: NULL timer");
    memset(tmr, 0, sizeof(*tmr));
    tmr->name = name;
    return RO_OK;
}

ro_status_t ro_timer_start_periodic(ro_timer_t* tmr, uint32_t period_us)
{
    RO_ASSERT(tmr != NULL, "ro_timer_start: NULL timer");
    tmr->period_us = period_us;
    get_data(tmr)->next_deadline_us = ro_time_us() + period_us;
    return RO_OK;
}

ro_status_t ro_timer_stop(ro_timer_t* tmr)
{
    if (tmr == NULL) return RO_EINVAL;
    tmr->period_us = 0;
    return RO_OK;
}

ro_status_t ro_timer_sync(ro_timer_t* tmr)
{
    if (tmr == NULL) return RO_EINVAL;
    if (tmr->period_us == 0) return RO_ENOTREADY;

    timer_host_data_t* d = get_data(tmr);
    uint64_t now = ro_time_us();

    if (now < d->next_deadline_us) {
        uint64_t remaining_us = d->next_deadline_us - now;
#ifdef _WIN32
        Sleep((DWORD)(remaining_us / 1000));
#else
        usleep((useconds_t)remaining_us);
#endif
    }

    d->next_deadline_us += tmr->period_us;
    return RO_OK;
}
