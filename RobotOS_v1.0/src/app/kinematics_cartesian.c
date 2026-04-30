/* kinematics_cartesian.c — Cartesian Kinematics Implementation
 *
 * Trivial 1:1 axis mapping: step_count = round(delta_mm * steps_per_mm).
 */
#include "../../include/app/kinematics_cartesian.h"
#include <math.h>

ro_status_t kinematics_cart_to_steps(const cart_pos_t* pos,
                                      float steps_per_mm_x,
                                      float steps_per_mm_y,
                                      float steps_per_mm_z,
                                      cart_steps_t* out)
{
    if (!pos || !out) return RO_EINVAL;
    out->sx = (int32_t)roundf(pos->x * steps_per_mm_x);
    out->sy = (int32_t)roundf(pos->y * steps_per_mm_y);
    out->sz = (int32_t)roundf(pos->z * steps_per_mm_z);
    return RO_OK;
}

ro_status_t kinematics_steps_to_cart(const cart_steps_t* steps,
                                      float steps_per_mm_x,
                                      float steps_per_mm_y,
                                      float steps_per_mm_z,
                                      cart_pos_t* out)
{
    if (!steps || !out) return RO_EINVAL;
    if (steps_per_mm_x == 0.0f || steps_per_mm_y == 0.0f || steps_per_mm_z == 0.0f)
        return RO_EINVAL;

    out->x = (float)steps->sx / steps_per_mm_x;
    out->y = (float)steps->sy / steps_per_mm_y;
    out->z = (float)steps->sz / steps_per_mm_z;
    return RO_OK;
}
