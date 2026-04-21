#include "fan.h"
#include "drivers/pwm/pwm.h"
#include "bsp/nano.h"

static uint8_t current_pwm_value = 0;

void Fan_Init(uint8_t initial_pwm) {
    GPIO_Init(PWM, GPIO_OUTPUT);
    PWM_Init(D10, 25000);

    current_pwm_value = initial_pwm;
    PWM_SetDutyCycle(D10, current_pwm_value);
}

void Fan_SetPWM(uint8_t pwm) {
    current_pwm_value = pwm;
    PWM_SetDutyCycle(D10, current_pwm_value);
}

uint8_t Fan_GetPWM(void) {
    return current_pwm_value;
}