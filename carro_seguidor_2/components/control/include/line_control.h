#ifndef LINE_CONTROL_H
#define LINE_CONTROL_H

typedef struct
{
    float kp;
    float ki_d;   /* Ki * T  — ganancia integral discreta */
    float kd_d;   /* Kd / T  — ganancia derivativa discreta */
    float alpha;  /* coeficiente filtro EMA derivada [0,1) */

    float integral;
    float prev_error;
    float filtered_diff;  /* diferencia filtrada e[k]-e[k-1] */

    float output_min;
    float output_max;

} line_control_t;

/*
 * kp, ki, kd: ganancias continuas equivalentes.
 * T: periodo de muestreo en segundos (constante de diseno).
 * alpha: coeficiente EMA del filtro de derivada.
 *        0 = sin filtro, valores cercanos a 1 = muy filtrado.
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
);

/* Calcula la accion de control discreta para el error actual e[k]. */
float line_control_compute(line_control_t *control, float error);

void line_control_reset(line_control_t *control);

#endif
