#ifndef INPUT_H
#define INPUT_H

#include <stdint.h>
#include "drivers/gpio/gpio.h"

#define SUS      GPIO_PORTD, 7
#define DREAPTA  GPIO_PORTD, 6
#define JOS      GPIO_PORTD, 5
#define STANGA   GPIO_PORTB, 0

void Input_Init(void);
int8_t Input_ReadSensor(void);

#endif