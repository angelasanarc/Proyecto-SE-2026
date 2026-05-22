#ifndef LINE_CONTROL_H
#define LINE_CONTROL_H

typedef struct
{
    float kp;
    float ki;
    float kd;

    float integral;
    float previous_error;

    float output_min;
    float output_max;

} line_control_t;

void line_control_init(
    line_control_t *control,
    float kp,
    float ki,
    float kd,
    float output_min,
    float output_max
);

float line_control_compute(
    line_control_t *control,
    float error,
    float dt
);

void line_control_reset(line_control_t *control);

#endif