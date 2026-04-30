/* kinematics_cartesian.h — Cartesian Kinematics (Application Layer)
 *
 * Converts Cartesian mm deltas to step counts and back.
 * This is the simplest kinematics model (1:1 axis mapping).
 */
#ifndef ROBOTOS_APP_KINEMATICS_CARTESIAN_H
#define ROBOTOS_APP_KINEMATICS_CARTESIAN_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Cartesian position in mm */
typedef struct {
    float x, y, z;
} cart_pos_t;

/* Step counts */
typedef struct {
    int32_t sx, sy, sz;
} cart_steps_t;

/* mm → steps */
ro_status_t kinematics_cart_to_steps(const cart_pos_t* pos,
                                      float steps_per_mm_x,
                                      float steps_per_mm_y,
                                      float steps_per_mm_z,
                                      cart_steps_t* out);

/* steps → mm */
ro_status_t kinematics_steps_to_cart(const cart_steps_t* steps,
                                      float steps_per_mm_x,
                                      float steps_per_mm_y,
                                      float steps_per_mm_z,
                                      cart_pos_t* out);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_APP_KINEMATICS_CARTESIAN_H */
