#include "servo.h"
#include "esp_err.h"
#include "driver/ledc.h"

/* ---------- Піни ---------- */
#define SERVO_GPIO          1

/* ---------- PWM (LEDC) ---------- */
#define SERVO_TIMER         LEDC_TIMER_0
#define SERVO_CHANNEL       LEDC_CHANNEL_0
#define SERVO_MODE          LEDC_LOW_SPEED_MODE
#define SERVO_FREQ_HZ       50                 // період 20 мс
#define SERVO_RES           LEDC_TIMER_14_BIT
#define SERVO_DUTY_MAX      ((1 << 14) - 1)    // 16383
#define SERVO_PERIOD_US     20000

/* ---------- Параметри SG90 ---------- */
#define SERVO_MIN_US        500
#define SERVO_MAX_US        2400
#define SERVO_MAX_ANGLE     180

/* ---------- Межі duty (рахуються з імпульсів) ---------- */
#define DUTY_MIN            ((SERVO_MIN_US * SERVO_DUTY_MAX) / SERVO_PERIOD_US)   // ≈ 409
#define DUTY_MAX            ((SERVO_MAX_US * SERVO_DUTY_MAX) / SERVO_PERIOD_US)   // ≈ 1965


/* ---------- Ініціалізація PWM для сервоприводу ---------- */
void servo_init(void)
{
    ledc_timer_config_t timer = {
        .speed_mode      = SERVO_MODE,
        .timer_num       = SERVO_TIMER,
        .duty_resolution = SERVO_RES,
        .freq_hz         = SERVO_FREQ_HZ,
        .clk_cfg         = LEDC_AUTO_CLK,
    };
    ESP_ERROR_CHECK(ledc_timer_config(&timer));

    ledc_channel_config_t channel = {
        .gpio_num   = SERVO_GPIO,
        .speed_mode = SERVO_MODE,
        .channel    = SERVO_CHANNEL,
        .timer_sel  = SERVO_TIMER,
        .duty       = DUTY_MIN,
        .hpoint     = 0,
    };
    ESP_ERROR_CHECK(ledc_channel_config(&channel));
}

/* static int pot_to_angle(int pot_val)
{
    if (pot_val < POT_RAW_MIN) pot_val = POT_RAW_MIN;
    if (pot_val > POT_RAW_MAX) pot_val = POT_RAW_MAX;
    return ((pot_val - POT_RAW_MIN) * POT_MAX_ANGLE) / (POT_RAW_MAX - POT_RAW_MIN);
} */
/* ---------- Перетворення кута в duty ---------- */
int angle_to_duty(int angle)
{
    if (angle < 0) angle = 0;
    if (angle > SERVO_MAX_ANGLE) angle = SERVO_MAX_ANGLE;
    return ((angle * (DUTY_MAX - DUTY_MIN)) / SERVO_MAX_ANGLE) + DUTY_MIN;
}

/* ---------- Встановлення ШІМ (duty cycle) для сервоприводу ---------- */
void servo_set_duty(int duty)
{
    ESP_ERROR_CHECK(ledc_set_duty(SERVO_MODE, SERVO_CHANNEL, (uint32_t)duty));
    ESP_ERROR_CHECK(ledc_update_duty(SERVO_MODE, SERVO_CHANNEL));
}

/* ---------- Встановлення сервоприводу в початкове положення ---------- */
void servo_set_to_start(void){
    int duty = angle_to_duty(INITIAL_ANGLE);  // Кут 90 градусів
    servo_set_duty(duty);
}
