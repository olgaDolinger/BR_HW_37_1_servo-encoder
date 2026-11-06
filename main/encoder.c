#include "encoder.h"
#include "esp_log.h"
#include "esp_err.h"
#include "driver/gpio.h"

// Тег для виводу повідомлень у лог
static const char *TAG = "ENCODER";

// Дескриптор апаратного лічильника імпульсів (PCNT)
pcnt_unit_handle_t pcnt_unit = NULL;


// Функція ініціалізації апаратного лічильника для енкодера
void setup_encoder_pcnt(void)
{
    // Межі лічильника (діапазон 16-бітного знакового числа)
    pcnt_unit_config_t unit_config = {
        .high_limit = 32767,    // Верхня межа лічильника
        .low_limit  = -32768,   // Нижня межа лічильника
    };
    // Створюємо блок лічильника PCNT
    ESP_ERROR_CHECK(pcnt_new_unit(&unit_config, &pcnt_unit));

    // Апаратний фільтр завад: імпульси коротші за 10 мкс ігноруються
    pcnt_glitch_filter_config_t filter_config = { .max_glitch_ns = 10000 };
    ESP_ERROR_CHECK(pcnt_unit_set_glitch_filter(pcnt_unit, &filter_config));

    // Режим X4: рахуємо обидва фронти обох сигналів.
    // Дребезг контактів дає пари +1/-1, які взаємно гасяться, тому значення не "стрибає".

    // Канал A: фронти сигналу A, напрямок за рівнем B
    pcnt_channel_handle_t chan_a = NULL;
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num  = ENCODER_GPIO_A,
        .level_gpio_num = ENCODER_GPIO_B,
    };
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &chan_a));

    /* Канал A: обидва фронти A, напрямок залежить від B */
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_a,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,     // Передній фронт A: +1
        PCNT_CHANNEL_EDGE_ACTION_DECREASE));   // Задній фронт A: -1
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,        // B = 1: напрямок як є
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE));   // B = 0: напрямок інвертується

    // Канал B: фронти сигналу B, напрямок за рівнем A
    pcnt_channel_handle_t chan_b = NULL;
    pcnt_chan_config_t chan_b_config = {
        .edge_gpio_num  = ENCODER_GPIO_B,
        .level_gpio_num = ENCODER_GPIO_A,
    };

    /* Канал B: обидва фронти B, напрямок залежить від A */
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_b_config, &chan_b));
    ESP_ERROR_CHECK(pcnt_channel_set_edge_action(chan_b,
        PCNT_CHANNEL_EDGE_ACTION_DECREASE,     // Передній фронт B: -1
        PCNT_CHANNEL_EDGE_ACTION_INCREASE));   // Задній фронт B: +1
    ESP_ERROR_CHECK(pcnt_channel_set_level_action(chan_b,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,        // A = 1: напрямок як є
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE));   // A = 0: напрямок інвертується

   /*  // Внутрішня підтяжка на A і B (на випадок, якщо на модулі немає резисторів)
    gpio_pullup_en(ENCODER_GPIO_A);
    gpio_pullup_en(ENCODER_GPIO_B); */

    pcnt_unit_enable(pcnt_unit);        // Вмикаємо блок лічильника
    pcnt_unit_clear_count(pcnt_unit);   // Скидаємо лічильник у 0
    pcnt_unit_start(pcnt_unit);         // Запускаємо рахунок

    ESP_LOGI(TAG, "PCNT X4 initialized");
}

// Функція ініціалізації піна кнопки
void setup_button(void)
{
    gpio_config_t btn_config = {
        .pin_bit_mask = 1ULL << ENCODER_GPIO_SW,   // Маска піна кнопки
        .mode         = GPIO_MODE_INPUT,           // Пін працює як вхід
        .pull_up_en   = GPIO_PULLUP_ENABLE,        // Внутрішня підтяжка до 3.3V (на KY-040 резистор для SW часто не впаяний)
        .pull_down_en = GPIO_PULLDOWN_DISABLE,     // Підтяжка до GND вимкнена
        .intr_type    = GPIO_INTR_DISABLE,         // Переривання не використовуємо, опитуємо в циклі
    };
    ESP_ERROR_CHECK(gpio_config(&btn_config));
}
