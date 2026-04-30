/* app_deadlines.h — Deadline IDs for the Application Layer */
#ifndef ROBOTOS_APP_DEADLINES_H
#define ROBOTOS_APP_DEADLINES_H

#include <robotos/ro_deadline.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Deadline identifiers — registered via ro_deadline_begin/end */
#define APP_DL_PLANNER_TICK       0x0100
#define APP_DL_PULSE_SERVICE      0x0101
#define APP_DL_CONTROL_RX_PARSE   0x0102

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_APP_DEADLINES_H */
