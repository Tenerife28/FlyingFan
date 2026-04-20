#ifndef LCD_H
#define LCD_H

#include <stdint.h>

#define LCD_ADDR (0x27 << 1)

void LCD_ShortDelay();
void LCD_LongDelay();
void LCD_WriteNibble(uint8_t nibble, uint8_t rs);
void LCD_Send(uint8_t val, uint8_t rs);
void LCD_Print(char *str);
void LCD_Init();


#endif