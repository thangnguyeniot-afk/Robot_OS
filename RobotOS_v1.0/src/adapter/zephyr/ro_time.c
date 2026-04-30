/* ============================================================================
 * ro_time.c — Zephyr Time Backend
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 * ========================================================================= */

#include <robotos/ro_time.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/kernel.h>
#endif

uint64_t ro_time_us(void)
{
#ifndef ROBOTOS_HOST_BUILD
    return k_ticks_to_us_floor64(k_uptime_ticks());
#else
    return 0;
#endif
}

uint32_t ro_time_ms(void)
{
#ifndef ROBOTOS_HOST_BUILD
    return (uint32_t)k_uptime_get();
#else
    return 0;
#endif
}

void ro_delay_us(uint32_t us)
{
#ifndef ROBOTOS_HOST_BUILD
    k_busy_wait(us);
#else
    (void)us;
#endif
}

void ro_delay_ms(uint32_t ms)
{
    ro_delay_us(ms * 1000u);
}
