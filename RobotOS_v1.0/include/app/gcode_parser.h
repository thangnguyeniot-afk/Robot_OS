/* gcode_parser.h — G-code Line Parser (Application Layer) */
#ifndef ROBOTOS_APP_GCODE_PARSER_H
#define ROBOTOS_APP_GCODE_PARSER_H

#include <robotos/ro_status.h>
#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* ── G-code command types ───────────────────────────────────── */
typedef enum {
    GCODE_NONE          = 0,
    GCODE_MOVE_RAPID,       /* G0  */
    GCODE_MOVE_LINEAR,      /* G1  */
    GCODE_HOME,             /* G28 */
    GCODE_SET_ABS,          /* G90 */
    GCODE_SET_REL,          /* G91 */
    GCODE_SPINDLE_ON,       /* M3  */
    GCODE_SPINDLE_OFF,      /* M5  */
    GCODE_EXTRUDER_TEMP     /* M104 */
} gcode_type_t;

/* Parsed command — pushed into cmd_q */
typedef struct {
    gcode_type_t type;
    float        x, y, z;    /* Coordinates / parameters      */
    float        f;           /* Feedrate (mm/min)             */
    float        s;           /* Spindle / temperature value   */
    bool         has_x, has_y, has_z;
    bool         has_f, has_s;
} gcode_cmd_t;

/* ── API ────────────────────────────────────────────────────── */

/* Parse a single null-terminated G-code line into cmd.
 * Returns RO_OK on success, RO_EINVAL on unrecognised command. */
ro_status_t gcode_parse_line(const char* line, gcode_cmd_t* cmd);

/* Reset parser state (absolute/relative mode, etc.) */
void gcode_parser_reset(void);

/* Query current coordinate mode */
bool gcode_parser_is_absolute(void);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_APP_GCODE_PARSER_H */
