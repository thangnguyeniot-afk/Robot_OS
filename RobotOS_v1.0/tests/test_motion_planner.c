/* test_motion_planner.c — Unit Tests for Motion Planner */
#include <stdio.h>
#include <string.h>
#include <assert.h>
#include <math.h>

#include "../../include/app/motion_planner.h"
#include "../../include/app/gcode_parser.h"
#include "../../include/app/motion_seg.h"
#include <robotos/ro_queue.h>
#include <robotos/ro_status.h>

#define TEST(name)  static void name(void)
#define RUN(name)   do { printf("  %-40s", #name); name(); printf("PASS\n"); } while(0)

/* ── Shared test fixtures ───────────────────────────────────── */
#define CMD_Q_DEPTH 8
#define SEG_Q_DEPTH 8

static gcode_cmd_t  s_cmd_buf[CMD_Q_DEPTH];
static motion_seg_t s_seg_buf[SEG_Q_DEPTH];
static ro_queue_t   s_cmd_q;
static ro_queue_t   s_seg_q;

static motion_planner_t* setup_planner(void)
{
    ro_queue_create(&s_cmd_q, s_cmd_buf, sizeof(gcode_cmd_t), CMD_Q_DEPTH);
    ro_queue_create(&s_seg_q, s_seg_buf, sizeof(motion_seg_t), SEG_Q_DEPTH);

    gcode_parser_reset();

    motion_planner_config_t cfg = {
        .cmd_q               = &s_cmd_q,
        .seg_q               = &s_seg_q,
        .steps_per_mm_x      = 80.0f,
        .steps_per_mm_y      = 80.0f,
        .steps_per_mm_z      = 400.0f,
        .default_feedrate_mm_min = 3000.0f,
        .default_accel_mm_s2 = 500.0f,
    };
    return motion_planner_create(&cfg);
}

/* ── Tests ──────────────────────────────────────────────────── */

TEST(test_empty_queue_returns_eagain)
{
    motion_planner_t* mp = setup_planner();
    assert(motion_planner_tick(mp) == RO_EAGAIN);
    motion_planner_destroy(mp);
}

TEST(test_linear_move_produces_segment)
{
    motion_planner_t* mp = setup_planner();

    gcode_cmd_t cmd;
    gcode_parse_line("G1 X10 Y5 F600", &cmd);
    ro_queue_send(&s_cmd_q, &cmd, 0);

    assert(motion_planner_tick(mp) == RO_OK);

    motion_seg_t seg;
    assert(ro_queue_recv(&s_seg_q, &seg, 0) == RO_OK);

    /* 10mm * 80 steps/mm = 800 steps */
    assert(seg.dx_steps == 800);
    /* 5mm * 80 steps/mm = 400 steps */
    assert(seg.dy_steps == 400);
    assert(seg.dz_steps == 0);
    assert(seg.axis_mask == (APP_AXIS_X | APP_AXIS_Y));
    assert(seg.f_steps_per_s > 0);

    motion_planner_destroy(mp);
}

TEST(test_rapid_move_max_speed)
{
    motion_planner_t* mp = setup_planner();

    gcode_cmd_t cmd;
    gcode_parse_line("G0 X100", &cmd);
    ro_queue_send(&s_cmd_q, &cmd, 0);
    motion_planner_tick(mp);

    motion_seg_t seg;
    ro_queue_recv(&s_seg_q, &seg, 0);
    /* Rapid should request max speed */
    assert(seg.f_steps_per_s == UINT32_MAX);

    motion_planner_destroy(mp);
}

TEST(test_position_tracking)
{
    motion_planner_t* mp = setup_planner();

    gcode_cmd_t cmd;
    gcode_parse_line("G1 X10 Y20 Z1 F600", &cmd);
    ro_queue_send(&s_cmd_q, &cmd, 0);
    motion_planner_tick(mp);

    float x, y, z;
    motion_planner_get_position(mp, &x, &y, &z);
    assert(fabsf(x - 10.0f) < 0.01f);
    assert(fabsf(y - 20.0f) < 0.01f);
    assert(fabsf(z - 1.0f)  < 0.01f);

    motion_planner_destroy(mp);
}

TEST(test_home_resets_position)
{
    motion_planner_t* mp = setup_planner();

    /* Move first */
    gcode_cmd_t cmd;
    gcode_parse_line("G1 X50 Y50", &cmd);
    ro_queue_send(&s_cmd_q, &cmd, 0);
    motion_planner_tick(mp);

    /* Drain seg_q */
    motion_seg_t seg;
    ro_queue_recv(&s_seg_q, &seg, 0);

    /* Home */
    gcode_parse_line("G28", &cmd);
    ro_queue_send(&s_cmd_q, &cmd, 0);
    motion_planner_tick(mp);

    float x, y, z;
    motion_planner_get_position(mp, &x, &y, &z);
    assert(fabsf(x) < 0.01f);
    assert(fabsf(y) < 0.01f);
    assert(fabsf(z) < 0.01f);

    motion_planner_destroy(mp);
}

TEST(test_relative_mode)
{
    motion_planner_t* mp = setup_planner();

    gcode_cmd_t cmd;
    /* Absolute move to (10, 10) */
    gcode_parse_line("G1 X10 Y10 F600", &cmd);
    ro_queue_send(&s_cmd_q, &cmd, 0);
    motion_planner_tick(mp);

    motion_seg_t seg;
    ro_queue_recv(&s_seg_q, &seg, 0);

    /* Switch to relative */
    gcode_parse_line("G91", &cmd);
    ro_queue_send(&s_cmd_q, &cmd, 0);
    motion_planner_tick(mp);

    /* Relative move (+5, +5) */
    gcode_parse_line("G1 X5 Y5 F600", &cmd);
    ro_queue_send(&s_cmd_q, &cmd, 0);
    motion_planner_tick(mp);
    ro_queue_recv(&s_seg_q, &seg, 0);

    /* Should be +5mm = 400 steps */
    assert(seg.dx_steps == 400);
    assert(seg.dy_steps == 400);

    float x, y, z;
    motion_planner_get_position(mp, &x, &y, &z);
    assert(fabsf(x - 15.0f) < 0.01f);
    assert(fabsf(y - 15.0f) < 0.01f);

    /* Reset parser back to absolute */
    gcode_parser_reset();
    motion_planner_destroy(mp);
}

/* ── Runner ─────────────────────────────────────────────────── */

int main(void)
{
    printf("test_motion_planner:\n");

    RUN(test_empty_queue_returns_eagain);
    RUN(test_linear_move_produces_segment);
    RUN(test_rapid_move_max_speed);
    RUN(test_position_tracking);
    RUN(test_home_resets_position);
    RUN(test_relative_mode);

    printf("All motion_planner tests passed!\n");
    return 0;
}
