#include "drivers/gpio/gpio.h"
#include "drivers/timer/timer0.h"
#include "bsp/nano.h"
#include "drivers/interrupt/external_interrupt.h"

typedef enum {
    ROSU_MASINI,
    VERDE_MASINI,
    GALBEN_MASINI,
    VERDE_PIETONI_BLINK
} Stari;

volatile Stari stare=VERDE_MASINI;
volatile uint8_t apasare=0;

void buton(){
    if (stare == VERDE_MASINI){
            apasare=1;
    }
}

void semaforCuButon() {
    Timer0_Init();

    GPIO_Init(D11, GPIO_OUTPUT); 
    GPIO_Init(D6, GPIO_OUTPUT);  
    GPIO_Init(D4, GPIO_OUTPUT); 
    GPIO_Init(A0, GPIO_OUTPUT);  
    GPIO_Init(A4, GPIO_OUTPUT);  
    GPIO_Init(D2, GPIO_INPUT);   

    uint32_t timp_rosu_m = 10000;
    uint32_t timp_galben_m = 3000;
    uint32_t timp_verde_m = 8000;
    uint32_t timp_blink = 3000;

    uint32_t t0 = Millis();
    uint32_t t_blink = 0;
    stare = VERDE_MASINI;

    GPIO_Write(D4, GPIO_HIGH); //VerdeM
    GPIO_Write(A0, GPIO_HIGH); //RosuP
    GPIO_Write(D11, GPIO_LOW); //RrosuM
    GPIO_Write(D6, GPIO_LOW);  //GalbenM
    GPIO_Write(A4, GPIO_LOW);  //VerdeP
    GPIO_Write(D2, GPIO_HIGH); // pull-up intern

    ExtInt_Init(INT_0, EXT_INT_FALLING_EDGE, buton);

    while (1) {
        uint32_t timp_curent = Millis();

        switch (stare) {
            case VERDE_MASINI:
                if (apasare&&(timp_curent - t0 >= timp_verde_m)) {
                    apasare = 0;
                    t0 = timp_curent;
                    GPIO_Write(D4, GPIO_LOW);  // Oprim Verde masini
                    GPIO_Write(D6, GPIO_HIGH); // Aprindem Galben masini
                    stare = GALBEN_MASINI;
                }
                break;

            case GALBEN_MASINI:
                if (timp_curent - t0 >= timp_galben_m) {
                    t0 = timp_curent;
                    GPIO_Write(D6, GPIO_LOW);
                    GPIO_Write(D11, GPIO_HIGH);
                    GPIO_Write(A0, GPIO_LOW);
                    GPIO_Write(A4, GPIO_HIGH);
                    stare = ROSU_MASINI;
                }
                break;

            case ROSU_MASINI:
                if (timp_curent - t0 >= timp_rosu_m) {
                    t0 = timp_curent;
                    t_blink = timp_curent;
                    stare = VERDE_PIETONI_BLINK;
                }
                break;

            case VERDE_PIETONI_BLINK:
                if (timp_curent - t_blink >= 400) {
                    GPIO_Toggle(A4);
                    t_blink = timp_curent;
                }

                if (timp_curent - t0 >= timp_blink) {
                    t0 = timp_curent;
                    GPIO_Write(A4, GPIO_LOW);
                    GPIO_Write(A0, GPIO_HIGH);
                    GPIO_Write(D11, GPIO_LOW);
                    GPIO_Write(D4, GPIO_HIGH);
                    
                    apasare=0;
                    stare=VERDE_MASINI;
                }
                break;
        }
    }
}

int main(void) {
    semaforCuButon();
    return 0;
}