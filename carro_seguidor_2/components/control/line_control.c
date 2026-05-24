#include "line_control.h"

/*
 * PID discreto en forma posicional (dominio z).
 *
 * Ecuaciones de diferencias:
 *   P[k]   = kp * e[k]
 *   I[k]   = I[k-1] + ki_d * e[k]          ki_d = Ki * T
 *   diff   = alpha*diff[k-1] + (1-alpha)*(e[k]-e[k-1])   (filtro EMA)
 *   D[k]   = kd_d * diff                    kd_d = Kd / T
 *   u[k]   = P[k] + I[k] + D[k]
 *
 * Anti-windup: I[k] se actualiza solo cuando u[k] no esta saturado.
 */

void line_control_init(
    line_control_t *control,
    float kp,
    float ki,
    float kd,
    float T,
    float alpha,
    float output_min,
    float output_max
)
{
    control->kp    = kp;
    control->ki_d  = ki * T;
    control->kd_d  = (T > 0.0f) ? (kd / T) : 0.0f;
    control->alpha = alpha;

    control->integral      = 0.0f;
    control->prev_error    = 0.0f;
    control->filtered_diff = 0.0f;

    control->output_min = output_min;
    control->output_max = output_max;
}

float line_control_compute(line_control_t *control, float error)
{
    float P = control->kp * error;

    float raw_diff = error - control->prev_error;
    control->filtered_diff = control->alpha * control->filtered_diff
                           + (1.0f - control->alpha) * raw_diff;
    float D = control->kd_d * control->filtered_diff;

    float u = P + control->integral + D;

    /* Anti-windup: acumular integral solo si la salida no esta saturada */
    if (u > control->output_min && u < control->output_max)
    {
        control->integral += control->ki_d * error;
    }

    if (u > control->output_max) u = control->output_max;
    if (u < control->output_min) u = control->output_min;

    control->prev_error = error;

    return u;
}

void line_control_reset(line_control_t *control)
{
    control->integral      = 0.0f;
    control->prev_error    = 0.0f;
    control->filtered_diff = 0.0f;
}
