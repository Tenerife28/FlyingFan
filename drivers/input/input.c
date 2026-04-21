#include "input.h"

void Input_Init(void) {
    GPIO_Init(SUS, GPIO_INPUT);
    GPIO_Init(DREAPTA, GPIO_INPUT);
    GPIO_Init(JOS, GPIO_INPUT);
    GPIO_Init(STANGA, GPIO_INPUT);
}

int8_t Input_ReadSensor(void) {
    if (GPIO_Read(SUS) == GPIO_LOW) return 0;
    if (GPIO_Read(DREAPTA) == GPIO_LOW) return 1;
    if (GPIO_Read(JOS) == GPIO_LOW) return 2;
    if (GPIO_Read(STANGA) == GPIO_LOW) return 3;

    return -1;
}
