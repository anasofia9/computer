#include "gpio.h"
volatile unsigned int * const FSEL0 = (unsigned int*)0x20200000;
volatile unsigned int * const SET0 = (unsigned int*)0x2020001C;
volatile unsigned int * const CLR0 = (unsigned int*)0x20200028;
volatile unsigned int * const LEV0 = (unsigned int*)0x20200034;
const unsigned int USEFUL_MASK = 0x7;

void gpio_init(void) {
}

void gpio_set_function(unsigned int pin, unsigned int function) {
    if( pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST || function < GPIO_FUNC_INPUT || function > GPIO_FUNC_ALT3) return;
    int shiftval = (pin%10) * 3;
    int mask = USEFUL_MASK << shiftval;
    int num = pin/10;
    *(FSEL0 + num) = *(FSEL0 + num) & ~mask; //preserve other values
    *(FSEL0 + num) = *(FSEL0 + num) | function <<shiftval;
}

unsigned int gpio_get_function(unsigned int pin) {
    if(pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST) return GPIO_INVALID_REQUEST;
    int shiftval = (pin%10) * 3;
    int curr = 0;
    int num = pin/10;
    curr = *(FSEL0+num) >> shiftval;
    return curr & USEFUL_MASK;
}

void gpio_set_input(unsigned int pin) {
    gpio_set_function(pin, GPIO_FUNC_INPUT); //is this enum still defined in this file?
}

void gpio_set_output(unsigned int pin) {
   gpio_set_function(pin, GPIO_FUNC_OUTPUT);
}

void gpio_write(unsigned int pin, unsigned int value) {
    int num =0;
    if(pin > 31) num =1;
    if(value ==1){
        *(SET0 + num) = 1 << (pin%32);
    }
    if(value ==0){
        *(CLR0 + num) = 1 << (pin%32);
    }
    
}

unsigned int gpio_read(unsigned int pin) {
    if(pin < GPIO_PIN_FIRST || pin > GPIO_PIN_LAST) return GPIO_INVALID_REQUEST;
    int num = 0;
    if(pin > 31) num =1;
    int curr = *(LEV0+num) >> (pin%32);
    return curr & 0x1;

}
