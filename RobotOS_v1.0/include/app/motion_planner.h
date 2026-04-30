/* motion_planner.h — Motion Planner (Application Layer)
 *
 * Converts gcode_cmd_t from cmd_q into motion_seg_t pushed to seg_q.
 * Runs in the t_planner thread at RT_CONTROL priority.
 */
#ifndef ROBOTOS_APP_MOTION_PLANNER_H
#define ROBOTOS_APP_MOTION_PLANNER_H

#include <robotos/ro_status.h>
#include <robotos/ro_queue.h>
#include "motion_seg.h"
#include "gcode_parser.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    ro_queue_t* cmd_q;      /* Input:  gcode_cmd_t queue  */
    ro_queue_t* seg_q;      /* Output: motion_seg_t queue */
    float       steps_per_mm_x;
    float       steps_per_mm_y;
    float       steps_per_mm_z;
    float       default_feedrate_mm_min;
    float       default_accel_mm_s2;
} motion_planner_config_t;

typedef struct motion_planner motion_planner_t;

motion_planner_t* motion_planner_create(const motion_planner_config_t* cfg);
void              motion_planner_destroy(motion_planner_t* mp);

/* Process one command from cmd_q → produce segment(s) into seg_q.
 * Returns RO_OK on success, RO_EAGAIN if cmd_q was empty. */
ro_status_t motion_planner_tick(motion_planner_t* mp);

/* Get the current absolute machine position (in mm) */
void        motion_planner_get_position(const motion_planner_t* mp,
                                         float* x, float* y, float* z);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_APP_MOTION_PLANNER_H */
