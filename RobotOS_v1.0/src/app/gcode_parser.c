/* gcode_parser.c — G-code Line Parser Implementation
 *
 * Supports: G0, G1, G28, G90, G91, M3, M5, M104
 * Parameters: X, Y, Z, F, S
 * Comments: semicolons and parentheses stripped.
 */
#include "../../include/app/gcode_parser.h"
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

/* ── Internal state ─────────────────────────────────────────── */
static bool s_absolute = true;  /* G90 by default */

/* ── Helpers ────────────────────────────────────────────────── */

static const char* skip_spaces(const char* p)
{
    while (*p && (*p == ' ' || *p == '\t')) p++;
    return p;
}

static float parse_float(const char** pp)
{
    const char* p = *pp;
    char buf[32];
    int  i = 0;
    if (*p == '-' || *p == '+') buf[i++] = *p++;
    while (i < 30 && (isdigit((unsigned char)*p) || *p == '.')) {
        buf[i++] = *p++;
    }
    buf[i] = '\0';
    *pp = p;
    return (float)atof(buf);
}

static int parse_int(const char** pp)
{
    const char* p = *pp;
    char buf[16];
    int  i = 0;
    while (i < 14 && isdigit((unsigned char)*p)) buf[i++] = *p++;
    buf[i] = '\0';
    *pp = p;
    return atoi(buf);
}

/* ── Public API ─────────────────────────────────────────────── */

ro_status_t gcode_parse_line(const char* line, gcode_cmd_t* cmd)
{
    if (!line || !cmd) return RO_EINVAL;
    memset(cmd, 0, sizeof(*cmd));
    cmd->type = GCODE_NONE;

    const char* p = skip_spaces(line);

    /* Skip empty / comment-only lines */
    if (*p == '\0' || *p == ';' || *p == '(') return RO_OK;

    /* ── Parse command letter ───────────────────────────────── */
    char letter = (char)toupper((unsigned char)*p);
    p++;
    p = skip_spaces(p);
    int code = parse_int(&p);

    if (letter == 'G') {
        switch (code) {
        case  0: cmd->type = GCODE_MOVE_RAPID;  break;
        case  1: cmd->type = GCODE_MOVE_LINEAR; break;
        case 28: cmd->type = GCODE_HOME;        break;
        case 90: cmd->type = GCODE_SET_ABS; s_absolute = true;  break;
        case 91: cmd->type = GCODE_SET_REL; s_absolute = false; break;
        default: return RO_EINVAL;
        }
    } else if (letter == 'M') {
        switch (code) {
        case   3: cmd->type = GCODE_SPINDLE_ON;    break;
        case   5: cmd->type = GCODE_SPINDLE_OFF;   break;
        case 104: cmd->type = GCODE_EXTRUDER_TEMP; break;
        default: return RO_EINVAL;
        }
    } else {
        return RO_EINVAL;
    }

    /* ── Parse parameters ───────────────────────────────────── */
    while (*p) {
        p = skip_spaces(p);
        if (*p == ';' || *p == '(') break;  /* Start of comment */
        if (*p == '\0') break;

        char param = (char)toupper((unsigned char)*p);
        p++;
        float val = parse_float(&p);

        switch (param) {
        case 'X': cmd->x = val; cmd->has_x = true; break;
        case 'Y': cmd->y = val; cmd->has_y = true; break;
        case 'Z': cmd->z = val; cmd->has_z = true; break;
        case 'F': cmd->f = val; cmd->has_f = true; break;
        case 'S': cmd->s = val; cmd->has_s = true; break;
        default:
            /* Unknown parameter — skip */
            break;
        }
    }

    return RO_OK;
}

void gcode_parser_reset(void)
{
    s_absolute = true;
}

bool gcode_parser_is_absolute(void)
{
    return s_absolute;
}
