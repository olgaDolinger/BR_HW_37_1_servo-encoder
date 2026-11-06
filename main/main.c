#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "servo.h"
#include "buzzer.h"
#include "encoder.h"

#define LOOP_MS          10           // Період опитування кнопки, мс
#define PRINT_EVERY      10           // Виводити позицію кожні 10 ітерацій (10 * 10 мс = 100 мс)
#define DEBOUNCE_COUNT   3            // Скільки опитувань поспіль стан має триматись (антидребезг)

// Тег для виводу повідомлень у лог
static const char *TAG = "ENCODER";


#define BIG_ROTATION_ANGLE 10       // Великий кут повороту, градуси
#define SMALL_ROTATION_ANGLE 5      // Малий кут повороту, градуси

void app_main(void)
{
    // Ініціалізація баз
    buzzer_init();
    // Ініціалізуємо енкодер і кнопку
    setup_encoder_pcnt();
    setup_button();
    // Ініціалізація PWM для сервоприводу
    servo_init();
    // початкове положення сервоприводу 90 градусів (щоб можливо було крутити в обидві сторони)
    servo_set_to_start();

    int stable_level = 1;   // Підтверджений (стабільний) стан кнопки: 1 = відпущена
    int debounce_cnt = 0;   // Лічильник підтвердження нового стану
    int print_cnt    = 0;   // Лічильник для періодичного виводу позиції

    while (1) {
        // ---------- Обробка кнопки з антидребезгом ----------
        int level = gpio_get_level(ENCODER_GPIO_SW);   // Поточний рівень на піні SW

        if (level == stable_level) {
            // Стан не змінився, скидаємо лічильник підтвердження
            debounce_cnt = 0;
        } else if (++debounce_cnt >= DEBOUNCE_COUNT) {
            // Новий стан тримається достатньо довго: приймаємо його
            stable_level = level;
            debounce_cnt = 0;

            if (stable_level == 0) {
                // Кнопку НАТИСНУЛИ (рівень 0)
                ESP_LOGI(TAG, "Кнопка натиснута");
                pcnt_unit_clear_count(pcnt_unit);   // Приклад дії: скидаємо позицію в 0
            } else {
                // Кнопку ВІДПУСТИЛИ (рівень 1)
                ESP_LOGI(TAG, "Кнопка відпущена");
            }
        }

        // ---------- Крутимо сервопривід ----------
        if (++print_cnt >= PRINT_EVERY) {
            print_cnt = 0;

            int count = 0;
            pcnt_unit_get_count(pcnt_unit, &count);              // Зчитуємо лічильник
            float angle = count * BIG_ROTATION_ANGLE + INITIAL_ANGLE;  // Кут у градусах

            int duty        = angle_to_duty(angle);
            servo_set_duty(duty);

            ESP_LOGI(TAG, "pos=%d steps, angle=%.1f deg, duty=%d", count, angle, duty);
        }

        // Пауза до наступного опитування
        vTaskDelay(pdMS_TO_TICKS(LOOP_MS));
    }
}
