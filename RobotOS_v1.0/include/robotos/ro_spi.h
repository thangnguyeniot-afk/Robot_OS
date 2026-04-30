/* ============================================================================
 * ro_spi.h — RobotOS SPI Bus Abstraction
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 * ========================================================================= */

#ifndef ROBOTOS_RO_SPI_H
#define ROBOTOS_RO_SPI_H

#include <robotos/ro_status.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct ro_spi_bus ro_spi_bus_t;

typedef struct {
    const char* dt_label;
    uint32_t    frequency_hz;   /* Clock frequency                     */
    uint8_t     mode;           /* SPI mode (0–3, CPOL|CPHA)           */
} ro_spi_config_t;

ro_spi_bus_t* ro_spi_get(const ro_spi_config_t* cfg);
void          ro_spi_put(ro_spi_bus_t* bus);

/**
 * Full-duplex transfer.
 * @param tx     Transmit buffer (NULL = send zeros)
 * @param rx     Receive buffer (NULL = discard received bytes)
 * @param len    Number of bytes to transfer
 */
ro_status_t ro_spi_transceive(ro_spi_bus_t* bus,
                               const uint8_t* tx,
                               uint8_t* rx,
                               uint32_t len);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_SPI_H */
