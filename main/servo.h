#pragma once


#define INITIAL_ANGLE   90   // Початковий кут сервоприводу, градуси

void servo_init(void);
int  angle_to_duty(int angle);
void servo_set_duty(int duty);
void servo_set_to_start(void);
