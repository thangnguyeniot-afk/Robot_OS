/*
 * robotos_platform_critical_host_stub.c
 * Host stub for platform critical-section boundary (Phase 5D).
 *
 * Implements the public platform critical API using deterministic counters.
 * No Zephyr. No hardware. No output.
 *
 * enter() increments enter_count and current_depth; updates max_depth;
 *         returns a token with monotonically increasing opaque id.
 * exit()  increments exit_count; decrements current_depth if depth > 0.
 *         Extra exit() at depth=0 is ignored (no underflow).
 */

#include "robotos_platform_critical.h"
#include "robotos_platform_critical_host_stub.h"

static unsigned int s_enter_count;
static unsigned int s_exit_count;
static unsigned int s_current_depth;
static unsigned int s_max_depth;
static uintptr_t    s_next_id;

void robotos_platform_critical_host_reset(void)
{
    s_enter_count   = 0u;
    s_exit_count    = 0u;
    s_current_depth = 0u;
    s_max_depth     = 0u;
    s_next_id       = 0u;
}

unsigned int robotos_platform_critical_host_enter_count(void)
{
    return s_enter_count;
}

unsigned int robotos_platform_critical_host_exit_count(void)
{
    return s_exit_count;
}

unsigned int robotos_platform_critical_host_current_depth(void)
{
    return s_current_depth;
}

unsigned int robotos_platform_critical_host_max_depth(void)
{
    return s_max_depth;
}

/* ---- Public platform API implementation ----------------------------------- */

robotos_platform_critical_token_t robotos_platform_critical_enter(void)
{
    robotos_platform_critical_token_t token;

    s_next_id++;
    token.opaque = s_next_id;

    s_enter_count++;
    s_current_depth++;
    if (s_current_depth > s_max_depth) {
        s_max_depth = s_current_depth;
    }

    return token;
}

void robotos_platform_critical_exit(robotos_platform_critical_token_t token)
{
    (void)token; /* token not inspected by stub; matched by caller convention */

    s_exit_count++;
    if (s_current_depth > 0u) {
        s_current_depth--;
    }
}
