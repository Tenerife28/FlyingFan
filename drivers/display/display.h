#ifndef DISPLAY_H
#define DISPLAY_H

#include <stdint.h>

void Display_Init(void);
void Display_ShowWelcome(uint8_t pwm);
void Display_ShowRPM(uint16_t rpm);
void Display_DrawProgressBar(uint8_t pwm, int8_t momentum);

#endif
