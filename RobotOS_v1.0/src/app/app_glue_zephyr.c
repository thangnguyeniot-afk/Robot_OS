/* app_glue_zephyr.c — Zephyr-Specific Board Initialization Glue
 *
 * This file contains Zephyr-specific initialization that cannot be
 * expressed through the portable adapter API alone, such as:
 *   - Board-level pin mux
 *   - Devicetree label binding
 *   - Zephyr shell command registration
 *
 * On host builds this file is NOT compiled.
 */

#ifdef CONFIG_ZEPHYR_KERNEL  /* Only compiled in Zephyr builds */

#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/shell/shell.h>
#include <robotos/ro_log.h>
#include <robotos/ro_queue.h>

#include "../../include/app/gcode_parser.h"
#include "../../include/app/app_sm.h"

/* ── External glue accessors ────────────────────────────────── */
extern ro_queue_t* app_glue_get_cmd_q(void);
extern app_sm_t*   app_glue_get_sm(void);
extern void        app_glue_robotos_init(void);

/* ══════════════════════════════════════════════════════════════
 *  Zephyr Shell Commands
 * ══════════════════════════════════════════════════════════════ */

static int cmd_gcode(const struct shell* sh, size_t argc, char** argv)
{
    if (argc < 2) {
        shell_print(sh, "Usage: gcode <line>");
        return -EINVAL;
    }

    /* Concatenate arguments into one line (shell splits by spaces) */
    static char line_buf[96];
    line_buf[0] = '\0';
    for (size_t i = 1; i < argc; i++) {
        if (i > 1) strcat(line_buf, " ");
        strncat(line_buf, argv[i], sizeof(line_buf) - strlen(line_buf) - 1);
    }

    gcode_cmd_t cmd;
    ro_status_t rc = gcode_parse_line(line_buf, &cmd);
    if (rc != RO_OK) {
        shell_error(sh, "Parse error: %d", rc);
        return -EINVAL;
    }

    rc = ro_queue_send(app_glue_get_cmd_q(), &cmd, 100);
    if (rc != RO_OK) {
        shell_error(sh, "Queue full: %d", rc);
        return -ENOSPC;
    }

    shell_print(sh, "OK (type=%d)", (int)cmd.type);
    return 0;
}

static int cmd_status(const struct shell* sh, size_t argc, char** argv)
{
    (void)argc; (void)argv;
    app_sm_t* sm = app_glue_get_sm();
    shell_print(sh, "State: %s", app_sm_state_name(app_sm_get_state(sm)));
    return 0;
}

static int cmd_home(const struct shell* sh, size_t argc, char** argv)
{
    (void)argc; (void)argv;
    app_sm_dispatch(app_glue_get_sm(), APP_CMD_HOME);
    shell_print(sh, "Homing...");
    return 0;
}

static int cmd_run(const struct shell* sh, size_t argc, char** argv)
{
    (void)argc; (void)argv;
    app_sm_dispatch(app_glue_get_sm(), APP_CMD_RUN);
    shell_print(sh, "Running.");
    return 0;
}

static int cmd_stop(const struct shell* sh, size_t argc, char** argv)
{
    (void)argc; (void)argv;
    app_sm_dispatch(app_glue_get_sm(), APP_CMD_STOP);
    shell_print(sh, "Stopped.");
    return 0;
}

SHELL_STATIC_SUBCMD_SET_CREATE(sub_robotos,
    SHELL_CMD(gcode,  NULL, "Send G-code line",   cmd_gcode),
    SHELL_CMD(status, NULL, "Show machine state",  cmd_status),
    SHELL_CMD(home,   NULL, "Start homing",        cmd_home),
    SHELL_CMD(run,    NULL, "Start running",        cmd_run),
    SHELL_CMD(stop,   NULL, "Stop machine",         cmd_stop),
    SHELL_SUBCMD_SET_END
);
SHELL_CMD_REGISTER(ro, &sub_robotos, "RobotOS commands", NULL);

/* ══════════════════════════════════════════════════════════════
 *  Zephyr main() — entry point
 * ══════════════════════════════════════════════════════════════ */

void main(void)
{
    RO_LOG_INFO("=== RobotOS v1.0 (Zephyr) ===");
    app_glue_robotos_init();
    /* Zephyr scheduler takes over; threads run */
}

#endif /* CONFIG_ZEPHYR_KERNEL */
