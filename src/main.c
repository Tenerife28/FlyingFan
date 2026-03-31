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

// --- Funcția pentru Bara de Progres (Curățată de text) ---
void drawProgressBar(uint8_t pwm, int8_t momentum) {
    // Calculăm câte blocuri pline avem din 16 posibile
    uint8_t blocks = (pwm * 16) / 255; 
    if (blocks > 16) blocks = 16;

    LCD_Send(0xC0, 0); // Mutăm fix pe Linia 2
    
    // Desenăm blocurile pline (0xFF) și spațiile goale
    for (uint8_t i = 0; i < 16; i++) {
        if (i < blocks) {
            LCD_Send(0xFF, 1); 
        } else {
            LCD_Send(' ', 1);  
        }
    }

    // Gestionăm Blink-ul hardware
    if (momentum != 0) {
        uint8_t blink_pos = 0;
        if (momentum > 0) {
            blink_pos = blocks;
            if (blink_pos > 15) blink_pos = 15;
        } else {
            blink_pos = (blocks > 0) ? (blocks - 1) : 0;
        }
        
        LCD_Send(0xC0 + blink_pos, 0); // Mutăm cursorul pe blocul activ
        LCD_Send(0x0D, 0);             // Blink ON
    } else {
        LCD_Send(0x0C, 0);             // Blink OFF (Bară statică)
    }
}

int main(void) {
    initPins();
    LCD_Init();
    
    uint8_t current_pwm = 128; // Pornim la 50%
    PWM_Init(D10, 25000); 
    PWM_SetDutyCycle(D10, current_pwm);

    char lcd_buffer[17]; 
    uint16_t rpm = 0;
    uint32_t last_rpm_time = Millis();
    
    static uint16_t rpm_history[5] = {0};
    static uint8_t history_idx = 0;
    static uint8_t is_first_read = 1;

    int8_t last_sensor = -1; 
    int8_t momentum = 0;     
    uint32_t last_move_time = Millis();

    // 1. Inițializăm ecranul complet
    LCD_Send(0x80, 0); 
    LCD_Print("RPM: 0          ");
    drawProgressBar(current_pwm, 0); // Desenăm prima oară bara stabilă

    sei(); 

    while(1) {
        // --- Citire senzori ---
        int8_t current_sensor = -1;
        if      (GPIO_Read(SUS) == GPIO_LOW)     current_sensor = 0;
        else if (GPIO_Read(DREAPTA) == GPIO_LOW) current_sensor = 1;
        else if (GPIO_Read(JOS) == GPIO_LOW)     current_sensor = 2;
        else if (GPIO_Read(STANGA) == GPIO_LOW)  current_sensor = 3;

        // --- Procesare Gesturi ---
        if (current_sensor != -1 && current_sensor != last_sensor) {
            uint32_t now = Millis();

            if (now - last_move_time > 800) {
                momentum = 0; // Gest prea lent, s-a rupt mișcarea
            } 
            else if (last_sensor != -1) {
                int8_t step = current_sensor - last_sensor;
                
                if (step < -2) step += 4;
                if (step >  2) step -= 4;

                int8_t actual_movement = 0;

                // Mișcare cursivă
                if (step == 1 || step == -1) {
                    momentum = step; 
                    actual_movement = step;
                } 
                // A sărit un senzor (Fault Tolerance activat)
                else if (step == 2 || step == -2) {
                    if (momentum != 0) actual_movement = momentum * 2; 
                }

                // Aplicăm matematica pe PWM
                if (actual_movement != 0) {
                    int16_t new_pwm = (int16_t)current_pwm + (actual_movement * 15);
                    
                    if (new_pwm > 255) new_pwm = 255;
                    if (new_pwm < 0)   new_pwm = 0;
                    
                    current_pwm = (uint8_t)new_pwm;
                    PWM_SetDutyCycle(D10, current_pwm);

                    // Desenăm bara de progres și activăm blink-ul în direcția respectivă
                    drawProgressBar(current_pwm, momentum);
                }
            }
            last_sensor = current_sensor;
            last_move_time = now;
        }

        // --- RPM și Oprirea Blink-ului ---
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

            // Dacă gestul s-a oprit de mai mult de 1.5 secunde, oprim blink-ul hardware
            // (transformăm bara într-un indicator curat, static)
            if (momentum != 0 && (now - last_move_time > 1500)) {
                momentum = 0;
                drawProgressBar(current_pwm, 0);
            }

            last_rpm_time = now;
        }

        for(volatile uint32_t i = 0; i < 50000; i++);
    }
    return 0;
}