#include <stdio.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"

#include "servo.h"
#include "buzzer.h"
#include "encoder.h"

#define LOOP_MS                 10           // Період опитування кнопки, мс
#define PRINT_EVERY             10           // Виводити позицію кожні 10 ітерацій (10 * 10 мс = 100 мс)
#define DEBOUNCE_COUNT          3            // Скільки опитувань поспіль стан має триматись (антидребезг)
#define BUTTON_PRESSED_LONG     100            // Скільки опитувань поспіль стан має триматись (антидребезг)

// Тег для виводу повідомлень у лог
static const char *TAG = "ENCODER";


#define BIG_ROTATION_ANGLE          10       // Великий кут повороту, градуси
#define SMALL_ROTATION_ANGLE        5        // Малий кут повороту, градуси

int_fast16_t current_rotation_angle = BIG_ROTATION_ANGLE;   // Поточний режим обертання: true = великий кут, false = малий кут

void toggle_rotation_angle(void) {
    current_rotation_angle = (current_rotation_angle == BIG_ROTATION_ANGLE) ? SMALL_ROTATION_ANGLE : BIG_ROTATION_ANGLE;
}

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
    int hold_cnt = 0;   // Лічильник тривалого утримання кнопки
    bool long_done = false;   // Флаг, що вказує, чи було зафіксовано довге утримання кнопки

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
                hold_cnt = 0;   // Скидаємо лічильник тривалого утримання кнопки
                long_done = false;   // Нове натискання: довге утримання ще не спрацювало
                // Кнопку НАТИСНУЛИ (рівень 0)
                ESP_LOGI(TAG, "Кнопка натиснута");
               // pcnt_unit_clear_count(pcnt_unit);   // Приклад дії: скидаємо позицію в 0
            } else {
                // Кнопку ВІДПУСТИЛИ (рівень 1)
                toggle_rotation_angle();  // Змінюємо режим обертання (великий/малий кут)
                ESP_LOGI(TAG, "Кнопка відпущена");
            }
        }
        
        // ---------- Довге утримання ----------
        if (stable_level == 0 && !long_done && ++hold_cnt >= BUTTON_PRESSED_LONG) {
            long_done = true;
            ESP_LOGI(TAG, "Кнопка утримується довго");
            pcnt_unit_clear_count(pcnt_unit);
            servo_set_to_start();
        }

        // ---------- Крутимо сервопривід ----------
        if (++print_cnt >= PRINT_EVERY) {
            print_cnt = 0;

            int count = 0;
            pcnt_unit_get_count(pcnt_unit, &count);              // Зчитуємо лічильник
            float angle = count * current_rotation_angle + INITIAL_ANGLE;  // Кут у градусах

            int duty = angle_to_duty(angle);
            servo_set_duty(duty);

            ESP_LOGI(TAG, "pos=%d steps, angle=%.1f deg, duty=%d", count, angle, duty);
        }

        // Пауза до наступного опитування
        vTaskDelay(pdMS_TO_TICKS(LOOP_MS));
    }
}
