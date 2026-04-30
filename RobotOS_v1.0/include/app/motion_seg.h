/* motion_seg.h — Motion Segment Type (Application Layer)
 *
 * 24-byte segment struct pushed through the seg_q IPC queue
 * from the motion planner to the pulse manager.
 */
#ifndef ROBOTOS_APP_MOTION_SEG_H
#define ROBOTOS_APP_MOTION_SEG_H

#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Axis bit-mask */
#define APP_AXIS_X  0x01
#define APP_AXIS_Y  0x02
#define APP_AXIS_Z  0x04

/* 24 bytes — fits in a cache-line friendly queue slot */
typedef struct {
    int32_t  dx_steps;            /* Steps on X axis            */
    int32_t  dy_steps;            /* Steps on Y axis            */
    int32_t  dz_steps;            /* Steps on Z axis            */
    uint8_t  axis_mask;           /* Combination of APP_AXIS_*  */
    uint8_t  _pad[3];
    uint32_t f_steps_per_s;       /* Feedrate (steps / second)  */
    uint32_t accel_steps_s2;      /* Acceleration (steps / s²)  */
} motion_seg_t;

_Static_assert(sizeof(motion_seg_t) == 24, "motion_seg_t must be 24 bytes");

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_APP_MOTION_SEG_H */
