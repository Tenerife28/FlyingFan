#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "bsp/nano.h"
#include "drivers/interrupt/external_interrupt.h"
#include "drivers/pwm/pwm.h"
#include <stdio.h>
#include <avr/io.h>
#include <avr/interrupt.h>

#define SCL      GPIO_PORTD, 2
#define SDA      GPIO_PORTD, 3
#define PWM      GPIO_PORTB, 2
#define RPM      GPIO_PORTB, 3
#define SUS      GPIO_PORTD, 7
#define DREAPTA  GPIO_PORTD, 6
#define JOS      GPIO_PORTD, 5
#define STANGA   GPIO_PORTB, 0

#define LCD_ADDR (0x27 << 1)

const uint8_t RPM_PULSES_PER_REV = 2; 

volatile uint16_t pulse_count = 0;

void I2C_Delay() {
    for(volatile uint8_t i = 0; i < 25; i++); 
}

void LCD_ShortDelay() {
    for(volatile uint16_t i = 0; i < 800; i++);
}

void LCD_LongDelay() {
    for(volatile uint32_t i = 0; i < 35000; i++);
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

ISR(PCINT0_vect) {
    static uint8_t last_state = 1; 
    uint8_t current_state = (PINB & (1 << PB3)) ? 1 : 0;
    
    if (!current_state && last_state) { 
        pulse_count++;
    }
    last_state = current_state;
}

void initPins(){
    Timer0_Init();
    
    GPIO_Init(PWM, GPIO_OUTPUT);
    
    GPIO_Init(RPM, GPIO_INPUT);
    GPIO_Write(RPM, GPIO_HIGH); 
    
    PCICR |= (1 << PCIE0);    
    PCMSK0 |= (1 << PCINT3);  
    
    GPIO_Init(SUS, GPIO_INPUT);
    GPIO_Init(DREAPTA, GPIO_INPUT);
    GPIO_Init(JOS, GPIO_INPUT);
    GPIO_Init(STANGA, GPIO_INPUT);
}

uint8_t current_pwm = 128;

void update_pwm(int8_t directie) {
    if (directie > 0) {
        if (current_pwm <= 220) current_pwm += 35; 
        else current_pwm = 255;
    } 
    else if (directie < 0) {
        if (current_pwm >= 35) current_pwm -= 35;
        else current_pwm = 0;
    }
    PWM_SetDutyCycle(D10, current_pwm);
}

int main(void) {
    initPins();
    LCD_Init();
    
    PWM_Init(D10, 25000); 
    PWM_SetDutyCycle(D10, current_pwm);

    char lcd_buffer[17]; 
    uint16_t rpm = 0;
    uint32_t last_rpm_time = Millis();
    
    static uint16_t rpm_history[5] = {0};
    static uint8_t history_idx = 0;
    static uint8_t is_first_read = 1;

    uint8_t last_sensor = 0;
    int8_t rotation_score = 0;
    uint32_t last_move_time = Millis();

    LCD_Send(0x80, 0); 
    LCD_Print("RPM: 0          ");

    sei(); 

    while(1) {
        uint8_t current_sensor = 0;
        if      (GPIO_Read(SUS) == GPIO_LOW)     current_sensor = 1;
        else if (GPIO_Read(DREAPTA) == GPIO_LOW) current_sensor = 2;
        else if (GPIO_Read(JOS) == GPIO_LOW)     current_sensor = 3;
        else if (GPIO_Read(STANGA) == GPIO_LOW)  current_sensor = 4;

        if (current_sensor != 0 && current_sensor != last_sensor) {
            uint32_t now = Millis();
            
            if (now - last_move_time > 1000) {
                rotation_score = 0;
            }
            last_move_time = now;

            if ((last_sensor == 1 && current_sensor == 2) ||
                (last_sensor == 2 && current_sensor == 3) ||
                (last_sensor == 3 && current_sensor == 4) ||
                (last_sensor == 4 && current_sensor == 1)) {
                
                if (rotation_score < 0) rotation_score = 0;
                rotation_score++;
            }
            else if ((last_sensor == 1 && current_sensor == 4) ||
                     (last_sensor == 4 && current_sensor == 3) ||
                     (last_sensor == 3 && current_sensor == 2) ||
                     (last_sensor == 2 && current_sensor == 1)) {
                
                if (rotation_score > 0) rotation_score = 0;
                rotation_score--;
            }

            last_sensor = current_sensor;

            LCD_Send(0xC0, 0);

            if (rotation_score >= 3) {
                update_pwm(1);
                rotation_score = 0;
                LCD_Print("Viteza ++       ");
            }
            else if (rotation_score <= -3) {
                update_pwm(-1);
                rotation_score = 0;
                LCD_Print("Viteza --       ");
            }
        }

        uint32_t now = Millis();
        uint32_t elapsed = now - last_rpm_time;

        if (elapsed >= 1000) { 
            cli();
            uint16_t pulses = pulse_count;
            pulse_count = 0;
            sei();

            rpm_history[history_idx] = (pulses * 30000UL) / elapsed; 
            
            if (is_first_read) {
                for(int j = 1; j < 5; j++) {
                    rpm_history[j] = rpm_history[0]; 
                }
                is_first_read = 0;
            }
            
            history_idx = (history_idx + 1) % 5;
            
            uint32_t avg = 0;
            for (int j = 0; j < 5; j++) {
                avg += rpm_history[j];
            }
            rpm = avg / 5;

            sprintf(lcd_buffer, "RPM: %-5u      ", rpm);
            LCD_Send(0x80, 0); 
            LCD_Print(lcd_buffer);

            if (now - last_move_time > 2000) {
                LCD_Send(0xC0, 0);
                LCD_Print("Gestioneaza...  ");
            }

            last_rpm_time = now;
        }

        for(volatile uint32_t i = 0; i < 50000; i++);
    }
    return 0;
}