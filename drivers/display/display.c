#include "display.h"
#include "drivers/lcd/lcd.h"
#include <stdio.h>

void Display_Init(void) {
    LCD_Init();
}

void Display_ShowWelcome(uint8_t pwm) {
    LCD_Send(0x80, 0);
    LCD_Print("RPM: 0          ");
    Display_DrawProgressBar(pwm, 0);
}

void Display_ShowRPM(uint16_t rpm) {
    char lcd_buffer[17];

    sprintf(lcd_buffer, "RPM: %-5u      ", rpm);
    LCD_Send(0x80, 0);
    LCD_Print(lcd_buffer);
}

void Display_DrawProgressBar(uint8_t pwm, int8_t momentum) {
    uint8_t blocks = (pwm * 16) / 255;
    if (blocks > 16) blocks = 16;

    LCD_Send(0xC0, 0);

    for (uint8_t i = 0; i < 16; i++) {
        if (i < blocks) {
            LCD_Send(0xFF, 1);
        } else {
            LCD_Send(' ', 1);
        }
    }

    if (momentum != 0) {
        uint8_t blink_pos = 0;

        if (momentum > 0) {
            blink_pos = blocks;
            if (blink_pos > 15) blink_pos = 15;
        } else {
            blink_pos = (blocks > 0) ? (blocks - 1) : 0;
        }

        LCD_Send(0xC0 + blink_pos, 0);
        LCD_Send(0x0D, 0);
    } else {
        LCD_Send(0x0C, 0);
    }
}
