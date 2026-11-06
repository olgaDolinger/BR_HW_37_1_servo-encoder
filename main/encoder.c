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

    // Дескриптор каналу лічильника
    pcnt_channel_handle_t chan_a = NULL;
    // Канал рахує фронти сигналу A, а сигнал B визначає напрямок обертання
    pcnt_chan_config_t chan_a_config = {
        .edge_gpio_num  = ENCODER_GPIO_A,   // Пін, за фронтами якого ведеться рахунок
        .level_gpio_num = ENCODER_GPIO_B,   // Пін, за рівнем якого визначається напрямок
    };
    ESP_ERROR_CHECK(pcnt_new_channel(pcnt_unit, &chan_a_config, &chan_a));

    // Передній фронт A: +1, задній фронт A: ігнорувати
    pcnt_channel_set_edge_action(chan_a,
        PCNT_CHANNEL_EDGE_ACTION_INCREASE,
        PCNT_CHANNEL_EDGE_ACTION_HOLD);

    // B = 1: напрямок як є, B = 0: напрямок інвертується
    pcnt_channel_set_level_action(chan_a,
        PCNT_CHANNEL_LEVEL_ACTION_KEEP,
        PCNT_CHANNEL_LEVEL_ACTION_INVERSE);

    pcnt_unit_enable(pcnt_unit);        // Вмикаємо блок лічильника
    pcnt_unit_clear_count(pcnt_unit);   // Скидаємо лічильник у 0
    pcnt_unit_start(pcnt_unit);         // Запускаємо рахунок

    ESP_LOGI(TAG, "PCNT X1 initialized");
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
