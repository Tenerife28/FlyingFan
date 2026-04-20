#ifndef I2C_H
#define I2C_H

#include <stdint.h>
#include "drivers/gpio/gpio.h"

#define SCL GPIO_PORTD, 2
#define SDA GPIO_PORTD, 3


void I2C_Delay();
void I2C_Start();
void I2C_Stop();
void I2C_WriteByte(uint8_t data);



#endif // I2C_H