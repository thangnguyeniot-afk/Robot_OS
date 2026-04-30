/* pid.c — PID Controller Implementation (Framework Layer)
 *
 * Discrete PID with anti-windup (integral clamping).
 * Call pid_ctrl_update() at a fixed rate matching cfg.sample_time_s.
 */
#include <robotos/pid.h>
#include <string.h>

ro_status_t pid_ctrl_init(pid_ctrl_t* pid, const pid_ctrl_config_t* cfg)
{
    if (!pid || !cfg)           return RO_EINVAL;
    if (cfg->sample_time_s <= 0.0f) return RO_EINVAL;

    pid->cfg         = *cfg;
    pid->integral    = 0.0f;
    pid->prev_error  = 0.0f;
    pid->prev_output = 0.0f;
    return RO_OK;
}

float pid_ctrl_update(pid_ctrl_t* pid, float setpoint, float measurement)
{
    if (!pid) return 0.0f;

    float error = setpoint - measurement;
    float dt    = pid->cfg.sample_time_s;

    /* Proportional */
    float p_term = pid->cfg.kp * error;

    /* Integral with anti-windup clamp */
    pid->integral += pid->cfg.ki * error * dt;
    if (pid->integral > pid->cfg.integral_max)
        pid->integral = pid->cfg.integral_max;
    if (pid->integral < pid->cfg.integral_min)
        pid->integral = pid->cfg.integral_min;

    /* Derivative (on error, filtered) */
    float d_term = 0.0f;
    if (dt > 0.0f) {
        d_term = pid->cfg.kd * (error - pid->prev_error) / dt;
    }

    float output = p_term + pid->integral + d_term;

    /* Output clamp */
    if (output > pid->cfg.output_max) output = pid->cfg.output_max;
    if (output < pid->cfg.output_min) output = pid->cfg.output_min;

    pid->prev_error  = error;
    pid->prev_output = output;
    return output;
}

void pid_ctrl_reset(pid_ctrl_t* pid)
{
    if (!pid) return;
    pid->integral    = 0.0f;
    pid->prev_error  = 0.0f;
    pid->prev_output = 0.0f;
}

ro_status_t pid_ctrl_set_gains(pid_ctrl_t* pid, float kp, float ki, float kd)
{
    if (!pid) return RO_EINVAL;
    pid->cfg.kp = kp;
    pid->cfg.ki = ki;
    pid->cfg.kd = kd;
    return RO_OK;
}
