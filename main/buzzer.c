#include "buzzer.h"
#include "esp_err.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "driver/ledc.h"

// ---------------- Налаштування пінів  базера ----------------
#define BUZZER_PIN          GPIO_NUM_2


#define LEDC_MODE           LEDC_LOW_SPEED_MODE
#define LEDC_TIMER          LEDC_TIMER_1      // Окремий таймер (TIMER_0 зайнятий сервоприводом)
#define LEDC_DUTY_RES       LEDC_TIMER_10_BIT // Роздільна здатність 10 біт (0 - 1023)
#define LEDC_CHANNEL        LEDC_CHANNEL_1    // Окремий канал (CHANNEL_0 зайнятий сервоприводом)
#define LEDC_DUTY           512               // 50% заповнення (меандр для максимальної гучності)
#define NOTE                262
#define NOTE_DURATION       500  // Тривалість ноти в мс


void buzzer_init(void)
{
    // 1. Налаштування таймера LEDC
    ledc_timer_config_t ledc_timer = {
        .speed_mode       = LEDC_MODE,
        .timer_num        = LEDC_TIMER,
        .duty_resolution  = LEDC_DUTY_RES,
        .freq_hz          = 1000, // Початкова частота
        .clk_cfg          = LEDC_AUTO_CLK
    };
    ESP_ERROR_CHECK(ledc_timer_config(&ledc_timer));

    // 2. Налаштування каналу LEDC
    ledc_channel_config_t ledc_channel = {
        .speed_mode     = LEDC_MODE,
        .channel        = LEDC_CHANNEL,
        .timer_sel      = LEDC_TIMER,
        .intr_type      = LEDC_INTR_DISABLE,
        .gpio_num       = BUZZER_PIN,
        .duty           = 0, // Початково вимкнено (duty = 0)
        .hpoint         = 0
    };
    ESP_ERROR_CHECK(ledc_channel_config(&ledc_channel));
}

void buzzer_alarm(void){
     // Змінюємо частоту ноти
    ledc_set_freq(LEDC_MODE, LEDC_TIMER, NOTE);
    // Встановлюємо 50% заповнення для генерації звуку
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, LEDC_DUTY);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);   // Застосовуємо duty (без цього звук не вмикається)
    // Тривалість ноти
    vTaskDelay(pdMS_TO_TICKS(NOTE_DURATION));
    // Вимикаємо сигнал після тривалості ноти
    ledc_set_duty(LEDC_MODE, LEDC_CHANNEL, 0);
    ledc_update_duty(LEDC_MODE, LEDC_CHANNEL);
}
