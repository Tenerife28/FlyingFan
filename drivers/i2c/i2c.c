#include "i2c.h"
#include <stdio.h>
#include <stdint.h>

void I2C_Delay() {
    for(volatile uint8_t i = 0; i < 25; i++); 
}


void I2C_Start() {
    GPIO_Write(SDA, GPIO_HIGH); GPIO_Write(SCL, GPIO_HIGH); I2C_Delay();
    GPIO_Write(SDA, GPIO_LOW); I2C_Delay();
    GPIO_Write(SCL, GPIO_LOW);
}

void I2C_Stop() {
    GPIO_Write(SDA, GPIO_LOW); I2C_Delay();
    GPIO_Write(SCL, GPIO_HIGH); I2C_Delay();
    GPIO_Write(SDA, GPIO_HIGH); I2C_Delay();
}

void I2C_WriteByte(uint8_t data) {
    for (int i = 0; i < 8; i++) {
        GPIO_Write(SDA, (data & 0x80) ? GPIO_HIGH : GPIO_LOW);
        I2C_Delay();
        GPIO_Write(SCL, GPIO_HIGH); I2C_Delay();
        GPIO_Write(SCL, GPIO_LOW); I2C_Delay();
        data <<= 1;
    }
    GPIO_Write(SDA, GPIO_HIGH); I2C_Delay();
    GPIO_Write(SCL, GPIO_HIGH); I2C_Delay();
    GPIO_Write(SCL, GPIO_LOW);
}

