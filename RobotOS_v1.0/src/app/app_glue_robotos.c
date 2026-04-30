/* app_glue_robotos.c — Application ↔ RobotOS Adapter Glue
 *
 * Creates IPC queues, threads, and deadlines using ro_* adapter API.
 * This file is used when running on the real RobotOS adapter (Zephyr
 * or any future adapter backend).
 *
 * Three threads:
 *   t_shell       — BACKGROUND priority, reads G-code lines → cmd_q
 *   t_planner     — RT_CONTROL priority, cmd_q → seg_q
 *   t_pulse_mgr   — RT_PULSE   priority, seg_q → step pulses
 */
#include <robotos/ro_thread.h>
#include <robotos/ro_queue.h>
#include <robotos/ro_timer.h>
#include <robotos/ro_deadline.h>
#include <robotos/ro_time.h>
#include <robotos/ro_log.h>
#include <robotos/ro_assert.h>

#include "../../include/app/gcode_parser.h"
#include "../../include/app/motion_planner.h"
#include "../../include/app/motion_seg.h"
#include "../../include/app/app_sm.h"
#include "../../include/app/app_deadlines.h"
#include "../../include/app/config_profiles.h"

#include <string.h>
#include <stdatomic.h>

/* ── Kconfig-like defaults (overridden by real Kconfig in Zephyr) ── */
#ifndef CONFIG_APP_CMD_QUEUE_DEPTH
#define CONFIG_APP_CMD_QUEUE_DEPTH        64
#endif
#ifndef CONFIG_APP_MAX_MOTION_SEGMENTS
#define CONFIG_APP_MAX_MOTION_SEGMENTS    64
#endif
#ifndef CONFIG_APP_EVT_QUEUE_DEPTH
#define CONFIG_APP_EVT_QUEUE_DEPTH        8
#endif
#ifndef CONFIG_APP_PLANNER_TICK_HZ
#define CONFIG_APP_PLANNER_TICK_HZ        1000
#endif
#ifndef CONFIG_APP_SHELL_STACK_SIZE
#define CONFIG_APP_SHELL_STACK_SIZE       1024
#endif
#ifndef CONFIG_APP_PLANNER_STACK_SIZE
#define CONFIG_APP_PLANNER_STACK_SIZE     2048
#endif
#ifndef CONFIG_APP_PULSE_MGR_STACK_SIZE
#define CONFIG_APP_PULSE_MGR_STACK_SIZE   1024
#endif
#ifndef CONFIG_APP_DEFAULT_PROFILE
#define CONFIG_APP_DEFAULT_PROFILE        0
#endif

/* ── Feed hold (C11 atomic, shared with ISR context) ────────── */
_Atomic int g_feed_hold = 0;

/* ── Static storage for IPC queues ──────────────────────────── */
static gcode_cmd_t  s_cmd_buf[CONFIG_APP_CMD_QUEUE_DEPTH];
static motion_seg_t s_seg_buf[CONFIG_APP_MAX_MOTION_SEGMENTS];

static ro_queue_t   s_cmd_q;
static ro_queue_t   s_seg_q;

/* ── Planner instance ───────────────────────────────────────── */
static motion_planner_t* s_planner;

/* ── App SM ─────────────────────────────────────────────────── */
static app_sm_t* s_app_sm;

/* ── Thread stacks ──────────────────────────────────────────── */
RO_THREAD_STACK_DEFINE(s_shell_stack,   CONFIG_APP_SHELL_STACK_SIZE);
RO_THREAD_STACK_DEFINE(s_planner_stack, CONFIG_APP_PLANNER_STACK_SIZE);
RO_THREAD_STACK_DEFINE(s_pulse_stack,   CONFIG_APP_PULSE_MGR_STACK_SIZE);

/* ── Timer ──────────────────────────────────────────────────── */
static ro_timer_t s_planner_timer;

/* ── Deadlines ──────────────────────────────────────────────── */
static ro_deadline_t s_dl_planner  = { .budget_us = 500, .name = "planner_tick" };
static ro_deadline_t s_dl_pulse    = { .budget_us = 100, .name = "pulse_svc" };

/* ══════════════════════════════════════════════════════════════
 *  Shell Thread — reads G-code lines (stub: simulated input)
 * ══════════════════════════════════════════════════════════════ */
static void shell_entry(void* arg)
{
    (void)arg;
    RO_LOG_INFO("shell: started");

    /* In a real system this would read from UART / USB CDC.
     * For now, we simply idle and wait for external cmd_q pushes. */
    while (1) {
        ro_thread_sleep_ms(100);
    }
}

/* ══════════════════════════════════════════════════════════════
 *  Planner Thread — converts cmd → segments at 1 kHz
 * ══════════════════════════════════════════════════════════════ */
static void planner_entry(void* arg)
{
    (void)arg;
    RO_LOG_INFO("planner: started");

    while (1) {
        ro_timer_sync(&s_planner_timer);

        if (atomic_load(&g_feed_hold)) continue;

        ro_deadline_begin(&s_dl_planner);
        ro_status_t rc = motion_planner_tick(s_planner);
        ro_deadline_end(&s_dl_planner);

        if (rc == RO_EAGAIN) {
            /* No commands — check if we should signal queue empty */
            if (app_sm_get_state(s_app_sm) == APP_STATE_RUNNING) {
                if (ro_queue_count(&s_seg_q) == 0) {
                    app_sm_dispatch(s_app_sm, APP_CMD_QUEUE_EMPTY);
                }
            }
        }
    }
}

/* ══════════════════════════════════════════════════════════════
 *  Pulse Manager Thread — pops segments and drives steppers
 * ══════════════════════════════════════════════════════════════ */
static void pulse_mgr_entry(void* arg)
{
    (void)arg;
    RO_LOG_INFO("pulse_mgr: started");

    while (1) {
        motion_seg_t seg;
        ro_status_t rc = ro_queue_recv(&s_seg_q, &seg, RO_QUEUE_WAIT_FOREVER);
        if (rc != RO_OK) continue;

        if (atomic_load(&g_feed_hold)) continue;

        ro_deadline_begin(&s_dl_pulse);

        /* In a full implementation, this would feed stepper_drv_load_segment
         * and tick the hardware timer. Simplified: just consume segments. */
        RO_LOG_DEBUG("pulse: seg dx=%d dy=%d dz=%d spd=%u",
                      seg.dx_steps, seg.dy_steps, seg.dz_steps,
                      seg.f_steps_per_s);

        ro_deadline_end(&s_dl_pulse);
    }
}

/* ══════════════════════════════════════════════════════════════
 *  Glue Init — called from app_main or Zephyr main()
 * ══════════════════════════════════════════════════════════════ */
void app_glue_robotos_init(void)
{
    /* ── Load profile ───────────────────────────────────────── */
    const config_profile_t* prof =
        config_profile_get((config_profile_id_t)CONFIG_APP_DEFAULT_PROFILE);
    RO_ASSERT(prof != NULL, "Invalid default profile");

    RO_LOG_INFO("Profile: %s (%d axes)", prof->name, prof->num_axes);

    /* ── Create IPC queues ──────────────────────────────────── */
    ro_queue_create(&s_cmd_q, s_cmd_buf, sizeof(gcode_cmd_t),
                     CONFIG_APP_CMD_QUEUE_DEPTH);
    ro_queue_create(&s_seg_q, s_seg_buf, sizeof(motion_seg_t),
                     CONFIG_APP_MAX_MOTION_SEGMENTS);

    /* ── Create planner ─────────────────────────────────────── */
    motion_planner_config_t mp_cfg = {
        .cmd_q               = &s_cmd_q,
        .seg_q               = &s_seg_q,
        .steps_per_mm_x      = prof->steps_per_mm_x,
        .steps_per_mm_y      = prof->steps_per_mm_y,
        .steps_per_mm_z      = prof->steps_per_mm_z,
        .default_feedrate_mm_min = prof->max_feedrate_mm_min,
        .default_accel_mm_s2 = prof->default_accel_mm_s2,
    };
    s_planner = motion_planner_create(&mp_cfg);
    RO_ASSERT(s_planner != NULL, "planner alloc failed");

    /* ── Create App SM ──────────────────────────────────────── */
    s_app_sm = app_sm_create();
    RO_ASSERT(s_app_sm != NULL, "app_sm alloc failed");

    /* ── Planner tick timer ─────────────────────────────────── */
    uint32_t tick_us = 1000000U / CONFIG_APP_PLANNER_TICK_HZ;
    ro_timer_init(&s_planner_timer, "plan_tmr", tick_us, NULL, NULL);

    /* ── Spawn threads ──────────────────────────────────────── */
    ro_thread_config_t t_shell_cfg = {
        .name      = "t_shell",
        .stack     = s_shell_stack,
        .stack_size = CONFIG_APP_SHELL_STACK_SIZE,
        .priority  = RO_PRIO_BACKGROUND,
        .entry     = shell_entry,
        .arg       = NULL,
    };
    ro_thread_create(&t_shell_cfg);

    ro_thread_config_t t_plan_cfg = {
        .name      = "t_planner",
        .stack     = s_planner_stack,
        .stack_size = CONFIG_APP_PLANNER_STACK_SIZE,
        .priority  = RO_PRIO_RT_CONTROL,
        .entry     = planner_entry,
        .arg       = NULL,
    };
    ro_thread_create(&t_plan_cfg);

    ro_thread_config_t t_pulse_cfg = {
        .name      = "t_pulse_mgr",
        .stack     = s_pulse_stack,
        .stack_size = CONFIG_APP_PULSE_MGR_STACK_SIZE,
        .priority  = RO_PRIO_RT_PULSE,
        .entry     = pulse_mgr_entry,
        .arg       = NULL,
    };
    ro_thread_create(&t_pulse_cfg);

    /* ── Boot complete ──────────────────────────────────────── */
    app_sm_dispatch(s_app_sm, APP_CMD_INIT_DONE);
    RO_LOG_INFO("app_glue_robotos: init complete, state=%s",
                 app_sm_state_name(app_sm_get_state(s_app_sm)));
}

/* Accessors for external modules */
ro_queue_t* app_glue_get_cmd_q(void) { return &s_cmd_q; }
ro_queue_t* app_glue_get_seg_q(void) { return &s_seg_q; }
app_sm_t*   app_glue_get_sm(void)    { return s_app_sm; }
