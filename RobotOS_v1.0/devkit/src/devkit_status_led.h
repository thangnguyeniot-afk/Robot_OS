/*
 * devkit_status_led.h
 * Owns led0 GPIO alias binding for the devkit status LED.
 * No app or core logic — init and toggle only.
 */

#ifndef DEVKIT_STATUS_LED_H
#define DEVKIT_STATUS_LED_H

int devkit_status_led_init(void);
int devkit_status_led_toggle(void);

#endif /* DEVKIT_STATUS_LED_H */
