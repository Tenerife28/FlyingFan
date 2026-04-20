#include "lcd.h"
#include "drivers/i2c/i2c.h"
#include "drivers/gpio/gpio.h"

void LCD_ShortDelay() {
    for(volatile uint16_t i = 0; i < 800; i++);
}

void LCD_LongDelay() {
    for(volatile uint32_t i = 0; i < 35000; i++);
}

void LCD_WriteNibble(uint8_t nibble, uint8_t rs) {
    uint8_t base = (nibble & 0xF0) | rs | 0x08; 
    
    I2C_Start();
    I2C_WriteByte(LCD_ADDR);
    
    I2C_WriteByte(base | 0x04);  
    I2C_Delay();
    I2C_WriteByte(base & ~0x04); 
    
    I2C_Stop();
}

void LCD_Send(uint8_t val, uint8_t rs) {
    LCD_WriteNibble(val & 0xF0, rs);
    LCD_WriteNibble((val << 4) & 0xF0, rs);
    
    if (val == 0x01 || val == 0x02) LCD_LongDelay();
    else LCD_ShortDelay();
}

void LCD_Print(char *str) {
    while(*str) {
        LCD_Send((uint8_t)*str++, 0x01); 
    }
}

void LCD_Init() {
    GPIO_Init(SDA, GPIO_OUTPUT);
    GPIO_Init(SCL, GPIO_OUTPUT);
    
    for(volatile uint32_t i = 0; i < 200000; i++);

    LCD_WriteNibble(0x30, 0);
    for(volatile uint32_t i = 0; i < 60000; i++); 
    LCD_WriteNibble(0x30, 0);
    for(volatile uint16_t i = 0; i < 2000; i++); 
    LCD_WriteNibble(0x30, 0);
    LCD_WriteNibble(0x20, 0); 

    LCD_Send(0x28, 0); 
    LCD_Send(0x0C, 0); 
    LCD_Send(0x06, 0); 
    LCD_Send(0x01, 0); 
}
