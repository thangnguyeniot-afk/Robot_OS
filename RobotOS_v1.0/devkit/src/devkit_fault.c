/*
 * devkit_fault.c
 * Announces fault visibility at boot. Provides optional test panic (off by default).
 * To enable test panic: define DEVKIT_FAULT_TEST before including devkit_fault.h.
 */

#include "devkit_fault.h"

#include <zephyr/kernel.h>
#include <zephyr/logging/log.h>

LOG_MODULE_REGISTER(devkit_fault, LOG_LEVEL_INF);

void devkit_fault_init(void)
{
	LOG_INF("fault visibility active");
}

#ifdef DEVKIT_FAULT_TEST
void devkit_fault_test_panic(void)
{
	LOG_ERR("deliberate panic triggered — DEVKIT_FAULT_TEST");
	k_panic();
}
#endif
