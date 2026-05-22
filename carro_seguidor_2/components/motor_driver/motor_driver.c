#include "motor_driver.h"

#include "driver/gpio.h"
#include "driver/ledc.h"

#include "robot_config.h"

#define PWM_FREQUENCY 1000
#define PWM_RESOLUTION LEDC_TIMER_8_BIT

#define PWM_TIMER LEDC_TIMER_0
#define PWM_MODE LEDC_LOW_SPEED_MODE

#define MOTOR_A_CHANNEL LEDC_CHANNEL_0
#define MOTOR_B_CHANNEL LEDC_CHANNEL_1

static int clamp_pwm(int speed)
{
    if (speed > MAX_SPEED)
    {
        return MAX_SPEED;
    }

    if (speed < -MAX_SPEED)
    {
        return -MAX_SPEED;
    }

    return speed;
}

static void set_motor_a_direction(int speed)
{
    int forward = speed >= 0;

#if MOTOR_A_INVERT
    forward = !forward;
#endif

    if (forward)
    {
        gpio_set_level(PIN_AIN1, 1);
        gpio_set_level(PIN_AIN2, 0);
    }
    else
    {
        gpio_set_level(PIN_AIN1, 0);
        gpio_set_level(PIN_AIN2, 1);
    }
}

static void set_motor_b_direction(int speed)
{
    int forward = speed >= 0;

#if MOTOR_B_INVERT
    forward = !forward;
#endif

    if (forward)
    {
        gpio_set_level(PIN_BIN1, 1);
        gpio_set_level(PIN_BIN2, 0);
    }
    else
    {
        gpio_set_level(PIN_BIN1, 0);
        gpio_set_level(PIN_BIN2, 1);
    }
}

esp_err_t motor_driver_init(void)
{
    gpio_config_t direction_config =
    {
        .pin_bit_mask =
            (1ULL << PIN_AIN1) |
            (1ULL << PIN_AIN2) |
            (1ULL << PIN_BIN1) |
            (1ULL << PIN_BIN2) |
            (1ULL << PIN_STBY),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };

    esp_err_t ret = gpio_config(&direction_config);

    if (ret != ESP_OK)
    {
        return ret;
    }

    gpio_set_level(PIN_STBY, 1);

    ledc_timer_config_t timer_config =
    {
        .speed_mode = PWM_MODE,
        .timer_num = PWM_TIMER,
        .duty_resolution = PWM_RESOLUTION,
        .freq_hz = PWM_FREQUENCY,
        .clk_cfg = LEDC_AUTO_CLK
    };

    ret = ledc_timer_config(&timer_config);

    if (ret != ESP_OK)
    {
        return ret;
    }

    ledc_channel_config_t motor_a_channel =
    {
        .gpio_num = PIN_PWMA,
        .speed_mode = PWM_MODE,
        .channel = MOTOR_A_CHANNEL,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };

    ret = ledc_channel_config(&motor_a_channel);

    if (ret != ESP_OK)
    {
        return ret;
    }

    ledc_channel_config_t motor_b_channel =
    {
        .gpio_num = PIN_PWMB,
        .speed_mode = PWM_MODE,
        .channel = MOTOR_B_CHANNEL,
        .timer_sel = PWM_TIMER,
        .duty = 0,
        .hpoint = 0
    };

    ret = ledc_channel_config(&motor_b_channel);

    if (ret != ESP_OK)
    {
        return ret;
    }

    return motor_driver_stop();
}

esp_err_t motor_driver_set_speed(int left_speed, int right_speed)
{
    left_speed = clamp_pwm(left_speed);
    right_speed = clamp_pwm(right_speed);

    gpio_set_level(PIN_STBY, 1);

    set_motor_a_direction(left_speed);
    set_motor_b_direction(right_speed);

    int left_duty = left_speed >= 0 ? left_speed : -left_speed;
    int right_duty = right_speed >= 0 ? right_speed : -right_speed;

    esp_err_t ret = ledc_set_duty(
        PWM_MODE,
        MOTOR_A_CHANNEL,
        left_duty
    );

    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = ledc_update_duty(
        PWM_MODE,
        MOTOR_A_CHANNEL
    );

    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = ledc_set_duty(
        PWM_MODE,
        MOTOR_B_CHANNEL,
        right_duty
    );

    if (ret != ESP_OK)
    {
        return ret;
    }

    ret = ledc_update_duty(
        PWM_MODE,
        MOTOR_B_CHANNEL
    );

    return ret;
}

esp_err_t motor_driver_stop(void)
{
    ledc_set_duty(PWM_MODE, MOTOR_A_CHANNEL, 0);
    ledc_update_duty(PWM_MODE, MOTOR_A_CHANNEL);

    ledc_set_duty(PWM_MODE, MOTOR_B_CHANNEL, 0);
    ledc_update_duty(PWM_MODE, MOTOR_B_CHANNEL);

    gpio_set_level(PIN_AIN1, 0);
    gpio_set_level(PIN_AIN2, 0);

    gpio_set_level(PIN_BIN1, 0);
    gpio_set_level(PIN_BIN2, 0);

    return ESP_OK;
}