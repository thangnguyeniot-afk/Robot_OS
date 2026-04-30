/* ============================================================================
 * ro_i2c.c — Zephyr I²C Backend
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 * ========================================================================= */

#include <robotos/ro_i2c.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/drivers/i2c.h>
#include <zephyr/device.h>
#endif

#include <string.h>

#define RO_I2C_POOL_SIZE 4

typedef struct ro_i2c_bus {
    bool in_use;
#ifndef ROBOTOS_HOST_BUILD
    const struct device* dev;
#endif
} ro_i2c_slot_t;

static ro_i2c_slot_t s_i2c_pool[RO_I2C_POOL_SIZE];

ro_i2c_bus_t* ro_i2c_get(const char* dt_label)
{
    RO_ASSERT(dt_label != NULL, "ro_i2c_get: NULL label");

    for (int i = 0; i < RO_I2C_POOL_SIZE; i++) {
        if (!s_i2c_pool[i].in_use) {
            ro_i2c_slot_t* slot = &s_i2c_pool[i];
            memset(slot, 0, sizeof(*slot));
            slot->in_use = true;

#ifndef ROBOTOS_HOST_BUILD
            slot->dev = device_get_binding(dt_label);
            if (slot->dev == NULL) {
                slot->in_use = false;
                return NULL;
            }
#endif
            return (ro_i2c_bus_t*)slot;
        }
    }
    return NULL;
}

void ro_i2c_put(ro_i2c_bus_t* bus)
{
    if (bus == NULL) return;
    ro_i2c_slot_t* slot = (ro_i2c_slot_t*)bus;
    memset(slot, 0, sizeof(*slot));
}

ro_status_t ro_i2c_write(ro_i2c_bus_t* bus, uint16_t addr,
                          const uint8_t* data, uint32_t len)
{
    RO_ASSERT(bus != NULL, "ro_i2c_write: NULL bus");

#ifndef ROBOTOS_HOST_BUILD
    ro_i2c_slot_t* slot = (ro_i2c_slot_t*)bus;
    int ret = i2c_write(slot->dev, data, len, addr);
    return (ret == 0) ? RO_OK : RO_EFAIL;
#else
    (void)addr; (void)data; (void)len;
    return RO_OK;
#endif
}

ro_status_t ro_i2c_read(ro_i2c_bus_t* bus, uint16_t addr,
                         uint8_t* data_out, uint32_t len)
{
    RO_ASSERT(bus != NULL, "ro_i2c_read: NULL bus");

#ifndef ROBOTOS_HOST_BUILD
    ro_i2c_slot_t* slot = (ro_i2c_slot_t*)bus;
    int ret = i2c_read(slot->dev, data_out, len, addr);
    return (ret == 0) ? RO_OK : RO_EFAIL;
#else
    (void)addr; (void)data_out; (void)len;
    return RO_OK;
#endif
}

ro_status_t ro_i2c_write_read(ro_i2c_bus_t* bus, uint16_t addr,
                               const uint8_t* wr_data, uint32_t wr_len,
                               uint8_t* rd_data, uint32_t rd_len)
{
    RO_ASSERT(bus != NULL, "ro_i2c_write_read: NULL bus");

#ifndef ROBOTOS_HOST_BUILD
    ro_i2c_slot_t* slot = (ro_i2c_slot_t*)bus;
    int ret = i2c_write_read(slot->dev, addr,
                              wr_data, wr_len, rd_data, rd_len);
    return (ret == 0) ? RO_OK : RO_EFAIL;
#else
    (void)addr; (void)wr_data; (void)wr_len; (void)rd_data; (void)rd_len;
    return RO_OK;
#endif
}
