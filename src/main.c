#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "bsp/nano.h"
#include "drivers/interrupt/external_interrupt.h"

// Definiții Pini
#define SCL      GPIO_PORTD, 2
#define SDA      GPIO_PORTD, 3
#define PWM      GPIO_PORTB, 2
#define RPM      GPIO_PORTB, 3
#define TEMP     GPIO_PORTB, 4
#define SUS      GPIO_PORTD, 7 // d7
#define DREAPTA  GPIO_PORTD, 6 // d6
#define JOS      GPIO_PORTD, 5 // d5
#define STANGA   GPIO_PORTB, 0 // d8

#define LCD_ADDR (0x27 << 1)

// --- Driver LCD I2C Bit-Banging ---

void I2C_Delay() {
    for(volatile uint16_t i = 0; i < 400; i++); 
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

void LCD_Raw(uint8_t data) {
    I2C_Start();
    I2C_WriteByte(LCD_ADDR);
    I2C_WriteByte(data | 0x08); // Backlight ON
    I2C_Stop();
}

void LCD_WriteNibble(uint8_t nibble, uint8_t rs) {
    uint8_t base = (nibble & 0xF0) | rs | 0x08;
    LCD_Raw(base | 0x04);   // EN = 1
    I2C_Delay();
    LCD_Raw(base & ~0x04);  // EN = 0
    I2C_Delay();
}

void LCD_Send(uint8_t val, uint8_t rs) {
    LCD_WriteNibble(val & 0xF0, rs);
    LCD_WriteNibble((val << 4) & 0xF0, rs);
    for(volatile uint32_t i = 0; i < 5000; i++); 
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
    for(volatile uint32_t i = 0; i < 40000; i++);
    LCD_WriteNibble(0x30, 0);
    for(volatile uint32_t i = 0; i < 10000; i++);
    LCD_WriteNibble(0x30, 0);
    LCD_WriteNibble(0x20, 0); 

    LCD_Send(0x28, 0); 
    LCD_Send(0x0C, 0); 
    LCD_Send(0x06, 0); 
    LCD_Send(0x01, 0); 
    for(volatile uint32_t i = 0; i < 50000; i++);
}

// --- Logica Aplicatiei ---

void initPins(){
    Timer0_Init();
    GPIO_Init(PWM, GPIO_OUTPUT);
    GPIO_Init(RPM, GPIO_INPUT);
    GPIO_Init(TEMP, GPIO_INPUT);
    
    // Initializare pini directie (modificat d5/d8)
    GPIO_Init(SUS, GPIO_INPUT);
    GPIO_Init(DREAPTA, GPIO_INPUT);
    GPIO_Init(JOS, GPIO_INPUT);
    GPIO_Init(STANGA, GPIO_INPUT);
}

int main(void) {
    initPins();
    LCD_Init();

    LCD_Send(0x80, 0); // Linia 1
    LCD_Print("Senzor Detectat:");

    while(1) {
        LCD_Send(0xC0, 0); // Linia 2

        if      (GPIO_Read(SUS) == GPIO_LOW)     LCD_Print("Buton: SUS      ");
        else if (GPIO_Read(JOS) == GPIO_LOW)     LCD_Print("Buton: JOS      ");
        else if (GPIO_Read(STANGA) == GPIO_LOW)  LCD_Print("Buton: STANGA   ");
        else if (GPIO_Read(DREAPTA) == GPIO_LOW) LCD_Print("Buton: DREAPTA  ");
        else                                     LCD_Print("Niciunul        ");

        // Loop delay
        for(volatile uint32_t i = 0; i < 100000; i++);
    }
    return 0;
}