#include "drivers/timer/timer0.h"
#include "drivers/interrupt/external_interrupt.h"
#include "drivers/input/input.h"
#include "drivers/fan/fan.h"
#include "drivers/rpm/rpm.h"
#include "drivers/display/display.h"
#include <stdint.h>
#include <avr/io.h>
#include <avr/interrupt.h>

ISR(PCINT0_vect) {
    static uint8_t last_state = 1;
    uint8_t current_state = (PINB & (1 << PB3)) ? 1 : 0;

    if (!current_state && last_state) {
        pulse_count++;
    }

    last_state = current_state;
}

int main(void) {
    Timer0_Init();
    Input_Init();
    RPM_Init();
    Fan_Init(128);
    Display_Init();
    Display_ShowWelcome(Fan_GetPWM());

    uint16_t rpm = 0;
    uint32_t last_rpm_time = Millis();

    int8_t last_sensor = -1;
    int8_t momentum = 0;
    uint32_t last_move_time = Millis();

    sei();

    while (1) {
        int8_t current_sensor = Input_ReadSensor();

        if (current_sensor != -1 && current_sensor != last_sensor) {
            uint32_t now = Millis();

            if (now - last_move_time > 800) {
                momentum = 0;
            } else if (last_sensor != -1) {
                int8_t step = current_sensor - last_sensor;

                if (step < -2) step += 4;
                if (step > 2) step -= 4;

                int8_t actual_movement = 0;

                if (step == 1 || step == -1) {
                    momentum = step;
                    actual_movement = step;
                } else if (step == 2 || step == -2) {
                    if (momentum != 0) actual_movement = momentum * 2;
                }

                if (actual_movement != 0) {
                    int16_t new_pwm = (int16_t)Fan_GetPWM() + (actual_movement * 15);

                    if (new_pwm > 255) new_pwm = 255;
                    if (new_pwm < 0) new_pwm = 0;

                    Fan_SetPWM((uint8_t)new_pwm);
                    Display_DrawProgressBar(Fan_GetPWM(), momentum);
                }
            }

            last_sensor = current_sensor;
            last_move_time = now;
        }

        uint32_t now = Millis();
        uint32_t elapsed = now - last_rpm_time;

        if (elapsed >= 1000) {
            cli();
            uint16_t pulses = pulse_count;
            pulse_count = 0;
            sei();

            pulse_count = pulses;
            rpm = RPM_Update(elapsed);
            pulse_count = 0;

            Display_ShowRPM(rpm);

            if (momentum != 0 && (now - last_move_time > 1500)) {
                momentum = 0;
                Display_DrawProgressBar(Fan_GetPWM(), 0);
            }

            last_rpm_time = now;
        }

        for (volatile uint32_t i = 0; i < 50000; i++);
    }

    return 0;
}