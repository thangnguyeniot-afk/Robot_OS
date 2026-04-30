/* app_main.c — Host Build Entry Point
 *
 * Used for host (desktop) builds only.
 * On Zephyr, the entry point is in app_glue_zephyr.c.
 *
 * This file provides a simple interactive loop that reads G-code
 * from stdin and pushes it into the cmd_q, exercising the full
 * pipeline: parser → planner → seg_q → pulse_mgr.
 */

#ifndef CONFIG_ZEPHYR_KERNEL  /* Host build only */

#include <stdio.h>
#include <string.h>

#include <robotos/ro_log.h>
#include <robotos/ro_queue.h>
#include <robotos/ro_time.h>
#include <robotos/ro_thread.h>

#include "../../include/app/gcode_parser.h"
#include "../../include/app/app_sm.h"

/* ── External glue accessors ────────────────────────────────── */
extern void        app_glue_robotos_init(void);
extern ro_queue_t* app_glue_get_cmd_q(void);
extern app_sm_t*   app_glue_get_sm(void);

int main(void)
{
    printf("=== RobotOS v1.0 (Host) ===\n");
    app_glue_robotos_init();

    /* Allow threads to start */
    ro_thread_sleep_ms(50);

    /* Transition: IDLE → HOMING → READY */
    app_sm_dispatch(app_glue_get_sm(), APP_CMD_HOME);
    ro_thread_sleep_ms(10);
    app_sm_dispatch(app_glue_get_sm(), APP_CMD_HOMING_DONE);
    ro_thread_sleep_ms(10);
    app_sm_dispatch(app_glue_get_sm(), APP_CMD_RUN);

    printf("Enter G-code lines (Ctrl+C to exit):\n");

    char line[96];
    while (fgets(line, sizeof(line), stdin)) {
        /* Strip trailing newline */
        size_t len = strlen(line);
        if (len > 0 && line[len - 1] == '\n') line[--len] = '\0';
        if (len == 0) continue;

        /* Quit command */
        if (strcmp(line, "quit") == 0 || strcmp(line, "exit") == 0) break;

        /* Status command */
        if (strcmp(line, "status") == 0) {
            printf("State: %s\n",
                   app_sm_state_name(app_sm_get_state(app_glue_get_sm())));
            continue;
        }

        /* Parse and enqueue */
        gcode_cmd_t cmd;
        ro_status_t rc = gcode_parse_line(line, &cmd);
        if (rc != RO_OK) {
            fprintf(stderr, "Parse error: %d\n", rc);
            continue;
        }

        rc = ro_queue_send(app_glue_get_cmd_q(), &cmd, 100);
        if (rc != RO_OK) {
            fprintf(stderr, "Queue full: %d\n", rc);
            continue;
        }

        printf("OK (type=%d)\n", (int)cmd.type);

        /* Give planner a moment to process */
        ro_thread_sleep_ms(5);
    }

    printf("Exiting.\n");
    return 0;
}

#endif /* !CONFIG_ZEPHYR_KERNEL */
