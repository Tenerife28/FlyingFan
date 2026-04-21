#ifndef FAN_H
#define FAN_H

#include <stdint.h>
#include "drivers/gpio/gpio.h"

#define PWM GPIO_PORTB, 2

void Fan_Init(uint8_t initial_pwm);
void Fan_SetPWM(uint8_t pwm);
uint8_t Fan_GetPWM(void);

#endif
