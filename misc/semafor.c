#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "bsp/nano.h"

enum {
    ROSU,
    VERDE,
    GALBEN
} culoare;

void semafor() {

    Timer0_Init();
    GPIO_Init(LED_BUILTIN, GPIO_OUTPUT);
    GPIO_Init(D11, GPIO_OUTPUT); 
    GPIO_Init(D6, GPIO_OUTPUT); 
    GPIO_Init(D2, GPIO_OUTPUT); 

    uint32_t timp_rosu = 10000;
    uint32_t timp_galben = 1000;
    uint32_t timp_verde = 5000;

    uint32_t t0 = Millis();
    culoare = ROSU;

    GPIO_Write(D11, GPIO_HIGH);
    GPIO_Write(D6, GPIO_LOW);
    GPIO_Write(D2, GPIO_LOW);

    while (1) {
        uint32_t timp_curent = Millis();


        switch (culoare) {
            
            case ROSU:
                if (timp_curent - t0 >= timp_rosu) {
                    t0 = timp_curent;
                    culoare = VERDE;
                    
                    GPIO_Write(D11, GPIO_LOW);
                    GPIO_Write(D2, GPIO_HIGH);
                    GPIO_Toggle(LED_BUILTIN);
                       
                }
                break;

            case VERDE:
                if (timp_curent - t0 >= timp_verde) {
                    t0 = timp_curent;
                    culoare = GALBEN;
                    
                    GPIO_Write(D2, GPIO_LOW);
                    GPIO_Write(D6, GPIO_HIGH);
                    GPIO_Toggle(LED_BUILTIN);
                }
                break;

            case GALBEN:
                if (timp_curent - t0 >= timp_galben) {
                    t0 = timp_curent;
                    culoare = ROSU;
                    
                    GPIO_Write(D6, GPIO_LOW); 
                    GPIO_Write(D11, GPIO_HIGH);
                    GPIO_Toggle(LED_BUILTIN);
                }
                break;
        }
    }
}

int main(void) {
    semafor();

    return 0;
}