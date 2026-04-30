/* ============================================================================
 * ro_spi.c — Zephyr SPI Backend
 * ============================================================================
 * Layer: Adapter / Zephyr backend
 * ========================================================================= */

#include <robotos/ro_spi.h>
#include <robotos/ro_assert.h>

#ifndef ROBOTOS_HOST_BUILD
#include <zephyr/drivers/spi.h>
#include <zephyr/device.h>
#endif

#include <string.h>

#define RO_SPI_POOL_SIZE 4

typedef struct ro_spi_bus {
    bool     in_use;
    uint32_t frequency_hz;
    uint8_t  mode;
#ifndef ROBOTOS_HOST_BUILD
    const struct device* dev;
    struct spi_config    spi_cfg;
#endif
} ro_spi_slot_t;

static ro_spi_slot_t s_spi_pool[RO_SPI_POOL_SIZE];

ro_spi_bus_t* ro_spi_get(const ro_spi_config_t* cfg)
{
    RO_ASSERT(cfg != NULL, "ro_spi_get: NULL config");

    for (int i = 0; i < RO_SPI_POOL_SIZE; i++) {
        if (!s_spi_pool[i].in_use) {
            ro_spi_slot_t* slot = &s_spi_pool[i];
            memset(slot, 0, sizeof(*slot));
            slot->in_use       = true;
            slot->frequency_hz = cfg->frequency_hz;
            slot->mode         = cfg->mode;

#ifndef ROBOTOS_HOST_BUILD
            slot->dev = device_get_binding(cfg->dt_label);
            if (slot->dev == NULL) {
                slot->in_use = false;
                return NULL;
            }
            slot->spi_cfg.frequency = cfg->frequency_hz;
            slot->spi_cfg.operation = SPI_WORD_SET(8) | SPI_TRANSFER_MSB;
            if (cfg->mode & 0x01) slot->spi_cfg.operation |= SPI_MODE_CPHA;
            if (cfg->mode & 0x02) slot->spi_cfg.operation |= SPI_MODE_CPOL;
#endif
            return (ro_spi_bus_t*)slot;
        }
    }
    return NULL;
}

void ro_spi_put(ro_spi_bus_t* bus)
{
    if (bus == NULL) return;
    ro_spi_slot_t* slot = (ro_spi_slot_t*)bus;
    memset(slot, 0, sizeof(*slot));
}

ro_status_t ro_spi_transceive(ro_spi_bus_t* bus,
                               const uint8_t* tx,
                               uint8_t* rx,
                               uint32_t len)
{
    RO_ASSERT(bus != NULL, "ro_spi_transceive: NULL bus");
    if (len == 0) return RO_OK;

#ifndef ROBOTOS_HOST_BUILD
    ro_spi_slot_t* slot = (ro_spi_slot_t*)bus;

    const struct spi_buf tx_buf = { .buf = (void*)tx, .len = len };
    const struct spi_buf_set tx_set = { .buffers = &tx_buf, .count = (tx ? 1 : 0) };

    const struct spi_buf rx_buf = { .buf = rx, .len = len };
    const struct spi_buf_set rx_set = { .buffers = &rx_buf, .count = (rx ? 1 : 0) };

    int ret = spi_transceive(slot->dev, &slot->spi_cfg,
                              tx ? &tx_set : NULL,
                              rx ? &rx_set : NULL);
    return (ret == 0) ? RO_OK : RO_EFAIL;
#else
    (void)tx; (void)rx; (void)len;
    return RO_OK;
#endif
}
