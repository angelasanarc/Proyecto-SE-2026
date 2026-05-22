#include "line_control.h"

void line_control_init(
    line_control_t *control,
    float kp,
    float ki,
    float kd,
    float output_min,
    float output_max
)
{
    control->kp = kp;
    control->ki = ki;
    control->kd = kd;

    control->integral = 0.0f;
    control->previous_error = 0.0f;

    control->output_min = output_min;
    control->output_max = output_max;
}

float line_control_compute(
    line_control_t *control,
    float error,
    float dt
)
{
    if (dt <= 0.0f)
    {
        dt = 0.001f;
    }

    float proportional = control->kp * error;

    control->integral += error * dt;

    float integral = control->ki * control->integral;

    float derivative = (error - control->previous_error) / dt;

    float derivative_term = control->kd * derivative;

    float output = proportional + integral + derivative_term;

    if (output > control->output_max)
    {
        output = control->output_max;
    }

    if (output < control->output_min)
    {
        output = control->output_min;
    }

    control->previous_error = error;

    return output;
}

void line_control_reset(line_control_t *control)
{
    control->integral = 0.0f;
    control->previous_error = 0.0f;
}