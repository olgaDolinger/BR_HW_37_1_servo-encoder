#pragma once

#include "driver/pulse_cnt.h"

// ---------------- Налаштування пінів і параметрів енкодера ----------------
#define ENCODER_GPIO_A   5            // Пін CLK (сигнал A)
#define ENCODER_GPIO_B   6            // Пін DT (сигнал B)
#define ENCODER_GPIO_SW  7            // Пін SW (кнопка енкодера), оберіть будь-який вільний
#define ENCODER_PPR      20           // Кількість імпульсів на один оберт (PPR) енкодера
#define ENCODER_CPR      ENCODER_PPR  // Режим X1: 1 імпульс = 1 крок, тому CPR = PPR

// Дескриптор апаратного лічильника імпульсів (PCNT), визначений в encoder.c
extern pcnt_unit_handle_t pcnt_unit;

void setup_encoder_pcnt(void);
void setup_button(void);
