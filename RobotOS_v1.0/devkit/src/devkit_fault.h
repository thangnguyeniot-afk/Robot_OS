/*
 * devkit_fault.h
 * Minimal fault visibility interface for RobotOS devkit.
 * DEVKIT_FAULT_TEST must be defined explicitly to expose the test panic.
 */

#ifndef DEVKIT_FAULT_H
#define DEVKIT_FAULT_H

void devkit_fault_init(void);

#ifdef DEVKIT_FAULT_TEST
void devkit_fault_test_panic(void);
#endif

#endif /* DEVKIT_FAULT_H */
