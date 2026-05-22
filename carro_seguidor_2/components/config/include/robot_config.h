#ifndef ROBOT_CONFIG_H
#define ROBOT_CONFIG_H

/* OLED SSD1306 */
#define OLED_ADDR 0x3C
#define I2C_PORT 0
#define PIN_OLED_SDA 21
#define PIN_OLED_SCL 22

/* START */
#define PIN_START 34
#define START_ACTIVE_LEVEL 0

/* Sensores binarios */
#define NUM_SENSORS 8
#define DIGITAL_LINE_LEVEL 0

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

#define BASE_SPEED 120
#define MAX_SPEED 255
#define MIN_SPEED 0

#define CONTROL_KP 0.04f
#define CONTROL_KI 0.00f
#define CONTROL_KD 0.002f

#define CONTROL_OUTPUT_MIN -120.0f
#define CONTROL_OUTPUT_MAX  120.0f

#define CONTROL_PERIOD_MS 10

#endif