#include "../drivers/gpio/gpio.h"
#include "../drivers/interrupt/external_interrupt.h"
#include "../bsp/nano.h"

static void OnButtonPress(void) {
GPIO_Toggle(LED_BUILTIN);
}

int main(void) {
GPIO_Init(LED_BUILTIN, GPIO_OUTPUT);

GPIO_Init(D2, GPIO_INPUT);
GPIO_Write(D2, GPIO_HIGH); // pull-up intern

ExtInt_Init(INT_0, EXT_INT_FALLING_EDGE, OnButtonPress);

while (1) {
// Bucla principala ramane libera pentru alte task-uri.
}
}