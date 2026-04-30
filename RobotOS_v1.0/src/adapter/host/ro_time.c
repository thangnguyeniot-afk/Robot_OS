/* ============================================================================
 * ro_time.c — Host Time Backend (Windows / POSIX)
 * ============================================================================
 * Layer: Adapter / Host build
 * ========================================================================= */

#include <robotos/ro_time.h>

#ifdef _WIN32
    #include <windows.h>
    static LARGE_INTEGER s_freq;
    static LARGE_INTEGER s_start;
    static int s_init = 0;

    static void time_init_once(void) {
        if (!s_init) {
            QueryPerformanceFrequency(&s_freq);
            QueryPerformanceCounter(&s_start);
            s_init = 1;
        }
    }
#else
    #include <time.h>
    #include <unistd.h>
#endif

uint64_t ro_time_us(void)
{
#ifdef _WIN32
    time_init_once();
    LARGE_INTEGER now;
    QueryPerformanceCounter(&now);
    return (uint64_t)((now.QuadPart - s_start.QuadPart) * 1000000ULL
                      / s_freq.QuadPart);
#else
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return (uint64_t)ts.tv_sec * 1000000ULL + (uint64_t)ts.tv_nsec / 1000ULL;
#endif
}

uint32_t ro_time_ms(void)
{
    return (uint32_t)(ro_time_us() / 1000ULL);
}

void ro_delay_us(uint32_t us)
{
    /* Busy-wait for accuracy (same semantics as Zephyr k_busy_wait) */
    uint64_t target = ro_time_us() + us;
    while (ro_time_us() < target) {
        /* spin */
    }
}

void ro_delay_ms(uint32_t ms)
{
#ifdef _WIN32
    Sleep(ms);
#else
    usleep(ms * 1000u);
#endif
}
