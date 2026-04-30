/* ============================================================================
 * ro_gpio.h — RobotOS GPIO Abstraction
 * ============================================================================
 * Layer: Adapter (public header — NO <zephyr/*> includes)
 *
 * Provides basic GPIO operations: configure direction, read, write, and
 * register ISR callbacks for edge/level events.
 *
 * Device binding: GPIO instances are acquired by DT label via ro_gpio_get().
 * ========================================================================= */

#ifndef ROBOTOS_RO_GPIO_H
#define ROBOTOS_RO_GPIO_H

#include <robotos/ro_status.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ---- Direction ----------------------------------------------------------- */

typedef enum {
    RO_GPIO_DIR_INPUT  = 0,
    RO_GPIO_DIR_OUTPUT = 1,
} ro_gpio_dir_t;

/* ---- Interrupt trigger mode ---------------------------------------------- */

typedef enum {
    RO_GPIO_INT_EDGE_RISING  = 0,
    RO_GPIO_INT_EDGE_FALLING = 1,
    RO_GPIO_INT_EDGE_BOTH    = 2,
    RO_GPIO_INT_LEVEL_HIGH   = 3,
    RO_GPIO_INT_LEVEL_LOW    = 4,
} ro_gpio_int_mode_t;

/* ---- Configuration ------------------------------------------------------- */

typedef struct {
    const char* dt_label;     /* Device Tree label (e.g. "gpioa")    */
    uint32_t    pin;          /* Pin number within the GPIO port     */
    bool        active_high;  /* true = active-high, false = active-low */
} ro_gpio_config_t;

/* ---- GPIO handle (opaque) ------------------------------------------------ */

typedef struct ro_gpio ro_gpio_t;

/* ---- ISR callback type --------------------------------------------------- */

/**
 * GPIO interrupt callback — runs in ISR context.
 * MUST only call _isr-safe Adapter APIs (ro_queue_send_isr, etc.).
 */
typedef void (*ro_gpio_isr_t)(ro_gpio_t* gpio, void* user);

/* ---- Lifecycle ----------------------------------------------------------- */

ro_gpio_t*  ro_gpio_get(const ro_gpio_config_t* cfg);
void        ro_gpio_put(ro_gpio_t* gpio);

/* ---- Direction configuration --------------------------------------------- */

ro_status_t ro_gpio_set_dir(ro_gpio_t* gpio, ro_gpio_dir_t dir);

/* ---- Read / Write -------------------------------------------------------- */

/** Set output level (1 = logical high accounting for active_high). */
ro_status_t ro_gpio_set(ro_gpio_t* gpio, uint32_t value);

/** Read current input level (returns 0 or 1, accounting for active_high). */
int         ro_gpio_read(const ro_gpio_t* gpio);

/* ---- Interrupt ----------------------------------------------------------- */

ro_status_t ro_gpio_set_interrupt(ro_gpio_t* gpio,
                                  ro_gpio_int_mode_t mode,
                                  ro_gpio_isr_t cb,
                                  void* user);

ro_status_t ro_gpio_clear_interrupt(ro_gpio_t* gpio);

#ifdef __cplusplus
}
#endif

#endif /* ROBOTOS_RO_GPIO_H */
