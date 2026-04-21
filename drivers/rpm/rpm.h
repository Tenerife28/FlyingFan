#ifndef RPM_H
#define RPM_H

#include <stdint.h>
#include "drivers/gpio/gpio.h"

#define RPM GPIO_PORTB, 3

extern volatile uint16_t pulse_count;

void RPM_Init(void);
uint16_t RPM_Update(uint32_t elapsed);

#endif
