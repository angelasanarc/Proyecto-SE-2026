#ifndef WIFI_MONITOR_H
#define WIFI_MONITOR_H

#include <stdbool.h>
#include "esp_err.h"
#include "robot_config.h"

typedef struct {
    int   error;
    int   position;
    int   left_speed;
    int   right_speed;
    float correction;
    bool  line_detected;
    int   sensors[NUM_SENSORS];
    int   start_active;
    int   motors_on;
} robot_telemetry_t;

typedef struct {
    float kp;
    float ki;
    float kd;
    int   base_speed;
} robot_params_t;

esp_err_t wifi_monitor_init(void);
void      wifi_monitor_update(const robot_telemetry_t *telem);
void      wifi_monitor_get_params(robot_params_t *out);
bool      wifi_monitor_params_changed(robot_params_t *out);
void      wifi_monitor_publish_log(const char *msg);

#endif
