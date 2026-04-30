/* config_profiles.c — Machine Configuration Profiles Implementation */
#include "../../include/app/config_profiles.h"
#include <stddef.h>

static const config_profile_t s_profiles[PROFILE_COUNT] = {
    [PROFILE_CNC_2AXIS] = {
        .id                   = PROFILE_CNC_2AXIS,
        .name                 = "CNC 2-Axis",
        .num_axes             = 2,
        .steps_per_mm_x       = 80.0f,
        .steps_per_mm_y       = 80.0f,
        .steps_per_mm_z       = 0.0f,
        .max_feedrate_mm_min  = 3000.0f,
        .default_accel_mm_s2  = 500.0f,
        .has_spindle          = true,
        .has_extruder         = false,
    },
    [PROFILE_CNC_3AXIS] = {
        .id                   = PROFILE_CNC_3AXIS,
        .name                 = "CNC 3-Axis",
        .num_axes             = 3,
        .steps_per_mm_x       = 80.0f,
        .steps_per_mm_y       = 80.0f,
        .steps_per_mm_z       = 400.0f,
        .max_feedrate_mm_min  = 3000.0f,
        .default_accel_mm_s2  = 500.0f,
        .has_spindle          = true,
        .has_extruder         = false,
    },
    [PROFILE_PRINTER_2AXIS] = {
        .id                   = PROFILE_PRINTER_2AXIS,
        .name                 = "Printer 2-Axis",
        .num_axes             = 2,
        .steps_per_mm_x       = 80.0f,
        .steps_per_mm_y       = 80.0f,
        .steps_per_mm_z       = 0.0f,
        .max_feedrate_mm_min  = 6000.0f,
        .default_accel_mm_s2  = 1000.0f,
        .has_spindle          = false,
        .has_extruder         = true,
    },
    [PROFILE_PRINTER_3AXIS] = {
        .id                   = PROFILE_PRINTER_3AXIS,
        .name                 = "Printer 3-Axis",
        .num_axes             = 3,
        .steps_per_mm_x       = 80.0f,
        .steps_per_mm_y       = 80.0f,
        .steps_per_mm_z       = 400.0f,
        .max_feedrate_mm_min  = 6000.0f,
        .default_accel_mm_s2  = 1000.0f,
        .has_spindle          = false,
        .has_extruder         = true,
    },
};

const config_profile_t* config_profile_get(config_profile_id_t id)
{
    if ((int)id < 0 || id >= PROFILE_COUNT) return NULL;
    return &s_profiles[id];
}

const char* config_profile_name(config_profile_id_t id)
{
    const config_profile_t* p = config_profile_get(id);
    return p ? p->name : "Unknown";
}
