/* ============================================================================
 * ro_i2c.h — RobotOS I²C Bus Abstraction
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 * ========================================================================= */

#ifndef ROBOTOS_RO_I2C_H
#define ROBOTOS_RO_I2C_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ro_i2c_bus ro_i2c_bus_t;

ro_i2c_bus_t* ro_i2c_get(const char* dt_label);
void          ro_i2c_put(ro_i2c_bus_t* bus);

ro_status_t ro_i2c_write(ro_i2c_bus_t* bus, uint16_t addr,
                          const uint8_t* data, uint32_t len);

ro_status_t ro_i2c_read(ro_i2c_bus_t* bus, uint16_t addr,
                         uint8_t* data_out, uint32_t len);

ro_status_t ro_i2c_write_read(ro_i2c_bus_t* bus, uint16_t addr,
                               const uint8_t* wr_data, uint32_t wr_len,
                               uint8_t* rd_data, uint32_t rd_len);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_I2C_H */
