/* sensor.h — Generic Sensor API (Framework Layer) */
#ifndef ROBOTOS_SENSOR_H
#define ROBOTOS_SENSOR_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct sensor sensor_t;

typedef enum {
    SENSOR_TEMPERATURE = 0,
    SENSOR_HUMIDITY,
    SENSOR_PRESSURE,
    SENSOR_DISTANCE,
    SENSOR_CURRENT,
    SENSOR_VOLTAGE
} sensor_type_t;

typedef struct {
    sensor_type_t type;
    float         value;      /* SI unit (°C, Pa, m, A, V …) */
    uint64_t      timestamp_us;
} sensor_reading_t;

sensor_t*   sensor_get(const char* dt_label);
void        sensor_put(sensor_t* sensor);

/* Trigger a one-shot sample (non-blocking or blocking per driver) */
ro_status_t sensor_trigger(sensor_t* sensor);

/* Read the latest converted value */
ro_status_t sensor_read(sensor_t* sensor, sensor_reading_t* out);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_SENSOR_H */
