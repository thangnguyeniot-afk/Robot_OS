/* endstop.h — Endstop / Limit Switch API (Framework Layer) */
#ifndef ROBOTOS_ENDSTOP_H
#define ROBOTOS_ENDSTOP_H

#include <robotos/ro_status.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct endstop endstop_t;

/* Callback fires in ISR context */
typedef void (*endstop_trigger_cb_t)(endstop_t* endstop, void* user_data);

endstop_t*  endstop_get(const char* dt_label);
void        endstop_put(endstop_t* endstop);

bool        endstop_is_triggered(const endstop_t* endstop);
ro_status_t endstop_set_callback(endstop_t* endstop,
                                  endstop_trigger_cb_t cb, void* user_data);
ro_status_t endstop_clear_callback(endstop_t* endstop);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_ENDSTOP_H */
