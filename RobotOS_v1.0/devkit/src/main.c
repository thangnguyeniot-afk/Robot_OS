/*
 * devkit/src/main.c
 * RobotOS devkit — Phase 3A runtime skeleton entry point.
 * Thin: initializes runtime, enters runtime loop.
 */

#include "devkit_runtime.h"

int main(void)
{
	if (devkit_runtime_init() < 0) {
		return -1;
	}

	devkit_runtime_run(); /* does not return */
}
