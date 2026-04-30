/* pid.h — PID Controller (Framework Layer) */
#ifndef ROBOTOS_PID_H
#define ROBOTOS_PID_H

#include <robotos/ro_status.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float kp;
    float ki;
    float kd;
    float output_min;
    float output_max;
    float integral_min;
    float integral_max;
    float sample_time_s;     /* Expected call interval (for D term scaling) */
} pid_ctrl_config_t;

typedef struct {
    pid_ctrl_config_t cfg;
    float integral;
    float prev_error;
    float prev_output;
} pid_ctrl_t;

ro_status_t pid_ctrl_init(pid_ctrl_t* pid, const pid_ctrl_config_t* cfg);
float       pid_ctrl_update(pid_ctrl_t* pid, float setpoint, float measurement);
void        pid_ctrl_reset(pid_ctrl_t* pid);
ro_status_t pid_ctrl_set_gains(pid_ctrl_t* pid, float kp, float ki, float kd);

#ifdef __cplusplus
}
#endif
#endif /* ROBOTOS_PID_H */
