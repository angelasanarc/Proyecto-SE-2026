#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

/* OLED SSD1306 */
#define OLED_ADDR 0x3C
#define I2C_PORT 0
#define PIN_OLED_SDA 21
#define PIN_OLED_SCL 22

/* START */
#define PIN_START            34
#define START_ACTIVE_LEVEL   0
/* Ciclos consecutivos necesarios para aceptar cambio de estado del boton.
 * Con CONTROL_PERIOD_MS = 10 ms: 5 ciclos = 50 ms de debounce. */
#define BUTTON_DEBOUNCE_CYCLES 5

/* Sensores QTR-8RC */
#define NUM_SENSORS          8
#define SENSOR_CHARGE_US     10      /* tiempo de carga del capacitor en µs */
#define SENSOR_MAX_READ_US   2500    /* timeout de descarga (negro) en µs   */

/* Calibracion */
#define CALIBRATION_SAMPLES   800    /* muestras = 800 × 10 ms = 8 segundos */
#define CALIBRATION_THRESHOLD 500    /* umbral normalizado 0-1000 para linea */

/* TB6612FNG */
#define PIN_AIN1 2
#define PIN_AIN2 13
#define PIN_PWMA 12

#define PIN_BIN1 4
#define PIN_BIN2 5
#define PIN_PWMB 14

#define PIN_STBY 15

#define MOTOR_A_INVERT 0
#define MOTOR_B_INVERT 0

/* Control */
#define LINE_CENTER_POSITION 3500
#define LINE_MAX_POSITION 7000

#define BASE_SPEED 140
#define MAX_SPEED 255
#define MIN_SPEED -80

#define LOST_LINE_TIMEOUT_MS  1500  /* ms manteniendo ultima correccion si pierde la linea */
#define LOST_LINE_MAX_CYCLES  (LOST_LINE_TIMEOUT_MS / CONTROL_PERIOD_MS)


#define CONTROL_KP 0.15f
#define CONTROL_KI 0.05f
#define CONTROL_KD 0.001f

#define CONTROL_DERIVATIVE_ALPHA 0.3f

#define CONTROL_OUTPUT_MIN -220.0f
#define CONTROL_OUTPUT_MAX  220.0f

#define CONTROL_PERIOD_MS 10
#define CONTROL_PERIOD_S  (CONTROL_PERIOD_MS / 1000.0f)

/* WiFi Station — credenciales en sta_creds.h (no subir a git) */
#if defined(__has_include) && __has_include("sta_creds.h")
# include "sta_creds.h"
#else
# define STA_WIFI_SSID     "YOUR_WIFI_SSID"
# define STA_WIFI_PASSWORD "YOUR_WIFI_PASSWORD"
#endif

/* MQTT */
#define MQTT_BROKER_URI  "mqtt://broker.hivemq.com:1883"
#define MQTT_TELEM_TOPIC "se2026/robot/telem"
#define MQTT_CMD_TOPIC   "se2026/robot/cmd"

#endif