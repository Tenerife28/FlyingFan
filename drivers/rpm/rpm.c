#include "rpm.h"
#include <avr/io.h>

volatile uint16_t pulse_count = 0;

static uint16_t rpm_history[5] = {0};
static uint8_t history_idx = 0;
static uint8_t is_first_read = 1;

void RPM_Init(void) {
    GPIO_Init(RPM, GPIO_INPUT);
    GPIO_Write(RPM, GPIO_HIGH);

    PCICR |= (1 << PCIE0);
    PCMSK0 |= (1 << PCINT3);
}

uint16_t RPM_Update(uint32_t elapsed) {
    rpm_history[history_idx] = (pulse_count * 30000UL) / elapsed;

    if (is_first_read) {
        for (int j = 1; j < 5; j++) {
            rpm_history[j] = rpm_history[0];
        }
        is_first_read = 0;
    }

    history_idx = (history_idx + 1) % 5;

    uint32_t avg = 0;
    for (int j = 0; j < 5; j++) {
        avg += rpm_history[j];
    }

    return (uint16_t)(avg / 5);
}

