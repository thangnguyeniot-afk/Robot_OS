/* sensor.c — Generic Sensor Implementation (Framework Layer)
 *
 * Wraps adapter I2C/SPI/ADC read behind a uniform sensor_t interface.
 * Each sensor instance stores the latest reading in a local cache.
 */
#include <robotos/sensor.h>
#include <robotos/ro_i2c.h>
#include <robotos/ro_time.h>
#include <robotos/ro_mutex.h>
#include <robotos/ro_log.h>
#include <string.h>

#define SENSOR_MAX_INSTANCES  8

struct sensor {
    const char*      label;
    sensor_type_t    type;
    sensor_reading_t last;
    ro_mutex_t       mtx;
    bool             in_use;
};

static struct sensor s_pool[SENSOR_MAX_INSTANCES];

/* ── Public API ─────────────────────────────────────────────── */

sensor_t* sensor_get(const char* dt_label)
{
    for (int i = 0; i < SENSOR_MAX_INSTANCES; i++) {
        if (!s_pool[i].in_use) {
            memset(&s_pool[i], 0, sizeof(struct sensor));
            s_pool[i].label  = dt_label;
            s_pool[i].in_use = true;
            ro_mutex_create(&s_pool[i].mtx);
            return (sensor_t*)&s_pool[i];
        }
    }
    return NULL;
}

void sensor_put(sensor_t* sensor)
{
    struct sensor* s = (struct sensor*)sensor;
    if (!s) return;
    ro_mutex_destroy(&s->mtx);
    s->in_use = false;
}

ro_status_t sensor_trigger(sensor_t* sensor)
{
    struct sensor* s = (struct sensor*)sensor;
    if (!s) return RO_EINVAL;

    /* In a real driver this would initiate a bus transaction.
     * Here we simulate by timestamping a dummy read. */
    ro_mutex_lock(&s->mtx, RO_QUEUE_WAIT_FOREVER);
    s->last.timestamp_us = ro_time_us();
    /* s->last.value would be filled by actual hardware read */
    ro_mutex_unlock(&s->mtx);
    return RO_OK;
}

ro_status_t sensor_read(sensor_t* sensor, sensor_reading_t* out)
{
    struct sensor* s = (struct sensor*)sensor;
    if (!s || !out) return RO_EINVAL;

    ro_mutex_lock(&s->mtx, RO_QUEUE_WAIT_FOREVER);
    *out = s->last;
    ro_mutex_unlock(&s->mtx);
    return RO_OK;
}
