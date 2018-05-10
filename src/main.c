#include "printf.h"
#include "uart.h"
#include "gpio.h"
#include "display.h"
#include "glove.h"
#include "hand.h"
#include "preset.h"

void main(void) 
{
    init_glove();
    init_buttons();
    init_display();
    init_hand();
    while (1) {
        update_glove();
        update_display();
        if (PRESET_ON == preset_mode()) {
            to_preset();
        }
        else {
            update_hand();
       }
    }
}
