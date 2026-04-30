/* motion_planner.c — Motion Planner Implementation
 *
 * Reads gcode_cmd_t from cmd_q, converts coordinates to steps via
 * kinematics, pushes motion_seg_t into seg_q.
 *
 * Coordinate tracking:
 *   - Machine position kept in mm (float).
 *   - Absolute / relative mode handled by gcode_parser state.
 */
#include "../../include/app/motion_planner.h"
#include "../../include/app/kinematics_cartesian.h"
#include <robotos/ro_log.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

struct motion_planner {
    motion_planner_config_t cfg;
    /* Machine position in mm */
    float pos_x, pos_y, pos_z;
    /* Active feedrate */
    float feedrate_mm_min;
};

motion_planner_t* motion_planner_create(const motion_planner_config_t* cfg)
{
    if (!cfg || !cfg->cmd_q || !cfg->seg_q) return NULL;

    motion_planner_t* mp = (motion_planner_t*)calloc(1, sizeof(*mp));
    if (!mp) return NULL;
    mp->cfg = *cfg;
    mp->feedrate_mm_min = cfg->default_feedrate_mm_min;
    return mp;
}

void motion_planner_destroy(motion_planner_t* mp)
{
    free(mp);
}

ro_status_t motion_planner_tick(motion_planner_t* mp)
{
    if (!mp) return RO_EINVAL;

    gcode_cmd_t cmd;
    ro_status_t rc = ro_queue_recv(mp->cfg.cmd_q, &cmd, 0);
    if (rc != RO_OK) return RO_EAGAIN;

    switch (cmd.type) {
    case GCODE_NONE:
        return RO_OK;

    case GCODE_SET_ABS:
    case GCODE_SET_REL:
        /* Parser already handled mode switch */
        return RO_OK;

    case GCODE_SPINDLE_ON:
    case GCODE_SPINDLE_OFF:
    case GCODE_EXTRUDER_TEMP:
        /* Phase-2 peripherals — log only */
        RO_LOG_INFO("planner: peripheral cmd type=%d", (int)cmd.type);
        return RO_OK;

    case GCODE_HOME: {
        /* Reset position to origin */
        mp->pos_x = 0.0f;
        mp->pos_y = 0.0f;
        mp->pos_z = 0.0f;
        RO_LOG_INFO("planner: homed");
        return RO_OK;
    }

    case GCODE_MOVE_RAPID:
    case GCODE_MOVE_LINEAR:
        break;  /* Handle below */

    default:
        return RO_EINVAL;
    }

    /* ── Move command processing ────────────────────────────── */

    /* Update feedrate if specified */
    if (cmd.has_f && cmd.f > 0.0f) {
        mp->feedrate_mm_min = cmd.f;
    }

    /* Compute target position */
    float tgt_x = mp->pos_x;
    float tgt_y = mp->pos_y;
    float tgt_z = mp->pos_z;

    if (gcode_parser_is_absolute()) {
        if (cmd.has_x) tgt_x = cmd.x;
        if (cmd.has_y) tgt_y = cmd.y;
        if (cmd.has_z) tgt_z = cmd.z;
    } else {
        if (cmd.has_x) tgt_x += cmd.x;
        if (cmd.has_y) tgt_y += cmd.y;
        if (cmd.has_z) tgt_z += cmd.z;
    }

    /* Delta in mm */
    float dx_mm = tgt_x - mp->pos_x;
    float dy_mm = tgt_y - mp->pos_y;
    float dz_mm = tgt_z - mp->pos_z;

    /* Convert to steps via Cartesian kinematics */
    cart_pos_t cart_delta = { .x = dx_mm, .y = dy_mm, .z = dz_mm };
    cart_steps_t steps;
    kinematics_cart_to_steps(&cart_delta,
                              mp->cfg.steps_per_mm_x,
                              mp->cfg.steps_per_mm_y,
                              mp->cfg.steps_per_mm_z,
                              &steps);

    /* Build segment */
    motion_seg_t seg;
    memset(&seg, 0, sizeof(seg));
    seg.dx_steps = steps.sx;
    seg.dy_steps = steps.sy;
    seg.dz_steps = steps.sz;

    seg.axis_mask = 0;
    if (seg.dx_steps != 0) seg.axis_mask |= APP_AXIS_X;
    if (seg.dy_steps != 0) seg.axis_mask |= APP_AXIS_Y;
    if (seg.dz_steps != 0) seg.axis_mask |= APP_AXIS_Z;

    /* Feedrate: mm/min → steps/s (use dominant axis) */
    float mm_per_s = mp->feedrate_mm_min / 60.0f;
    float max_spm  = mp->cfg.steps_per_mm_x;
    if (mp->cfg.steps_per_mm_y > max_spm) max_spm = mp->cfg.steps_per_mm_y;
    if (mp->cfg.steps_per_mm_z > max_spm) max_spm = mp->cfg.steps_per_mm_z;
    seg.f_steps_per_s  = (uint32_t)(mm_per_s * max_spm);
    seg.accel_steps_s2 = (uint32_t)(mp->cfg.default_accel_mm_s2 * max_spm);

    if (seg.f_steps_per_s == 0) seg.f_steps_per_s = 1;

    /* Rapid: use max possible speed */
    if (cmd.type == GCODE_MOVE_RAPID) {
        seg.f_steps_per_s  = UINT32_MAX;  /* Pulse mgr will clamp */
        seg.accel_steps_s2 = UINT32_MAX;
    }

    /* Push to seg_q */
    rc = ro_queue_send(mp->cfg.seg_q, &seg, 2 /* ticks timeout */);
    if (rc != RO_OK) {
        RO_LOG_WARN("planner: seg_q full (%d)", rc);
        return rc;
    }

    /* Update machine position */
    mp->pos_x = tgt_x;
    mp->pos_y = tgt_y;
    mp->pos_z = tgt_z;

    return RO_OK;
}

void motion_planner_get_position(const motion_planner_t* mp,
                                  float* x, float* y, float* z)
{
    if (!mp) return;
    if (x) *x = mp->pos_x;
    if (y) *y = mp->pos_y;
    if (z) *z = mp->pos_z;
}
