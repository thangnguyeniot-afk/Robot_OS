/* config_profiles.h — Machine Configuration Profiles (Application Layer)
 *
 * Pre-built profiles for common robot configurations.
 */
#ifndef ROBOTOS_APP_CONFIG_PROFILES_H
#define ROBOTOS_APP_CONFIG_PROFILES_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    PROFILE_CNC_2AXIS      = 0,
    PROFILE_CNC_3AXIS      = 1,
    PROFILE_PRINTER_2AXIS  = 2,
    PROFILE_PRINTER_3AXIS  = 3,
    PROFILE_COUNT
} config_profile_id_t;

typedef struct {
    config_profile_id_t id;
    const char*         name;
    uint8_t             num_axes;
    float               steps_per_mm_x;
    float               steps_per_mm_y;
    float               steps_per_mm_z;
    float               max_feedrate_mm_min;
    float               default_accel_mm_s2;
    bool                has_spindle;
    bool                has_extruder;
} config_profile_t;

/* Get a built-in profile by ID (returns NULL if invalid) */
const config_profile_t* config_profile_get(config_profile_id_t id);

/* Get profile name string */
const char* config_profile_name(config_profile_id_t id);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_APP_CONFIG_PROFILES_H */
