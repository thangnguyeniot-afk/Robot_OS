/* ============================================================================
 * ro_timer.c — Zephyr Timer Backend
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 *
 * Implementation: wraps k_timer + k_timer_status_sync() for the
 * "timer-sync" pattern — blocking until next period expiry.
 * ========================================================================= */

#include <robotos/ro_timer.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/kernel.h>
#endif

#include <string.h>

/* ---- Internal: access the Zephyr timer stored inside _impl[] ------------- */

#ifndef ROBOTOS_HOST_BUILD
static inline struct k_timer* get_ktimer(ro_timer_t* tmr)
{
    /* _impl[] must be large enough for struct k_timer */
    return (struct k_timer*)tmr->_impl;
}
#endif

/* ---- API ----------------------------------------------------------------- */

ro_status_t ro_timer_init(ro_timer_t* tmr, const char* name)
{
    RO_ASSERT(tmr != NULL, "ro_timer_init: NULL timer");
    memset(tmr, 0, sizeof(*tmr));
    tmr->name = name;
    tmr->period_us = 0;

#ifndef ROBOTOS_HOST_BUILD
    k_timer_init(get_ktimer(tmr), NULL, NULL);
#endif

    return RO_OK;
}

ro_status_t ro_timer_start_periodic(ro_timer_t* tmr, uint32_t period_us)
{
    RO_ASSERT(tmr != NULL, "ro_timer_start: NULL timer");
    RO_ASSERT(period_us > 0, "ro_timer_start: zero period");

    tmr->period_us = period_us;

#ifndef ROBOTOS_HOST_BUILD
    k_timeout_t period = K_USEC(period_us);
    k_timer_start(get_ktimer(tmr), period, period);
#endif

    return RO_OK;
}

ro_status_t ro_timer_stop(ro_timer_t* tmr)
{
    if (tmr == NULL) return RO_EINVAL;
    tmr->period_us = 0;

#ifndef ROBOTOS_HOST_BUILD
    k_timer_stop(get_ktimer(tmr));
#endif

    return RO_OK;
}

ro_status_t ro_timer_sync(ro_timer_t* tmr)
{
    if (tmr == NULL) return RO_EINVAL;
    if (tmr->period_us == 0) return RO_ENOTREADY;

#ifndef ROBOTOS_HOST_BUILD
    /* Block until timer expires.  If the timer already fired (work overran),
     * returns immediately — the caller can detect this by measuring elapsed. */
    k_timer_status_sync(get_ktimer(tmr));
#endif

    return RO_OK;
}
