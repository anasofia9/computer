#include "preset.h"
#include "hand.h"
#include "gpio.h"

static int PUSHED  = 0;
static int RELEASED  = 1;

const unsigned int BUTTONS[5] = {GPIO_PIN0, GPIO_PIN5, GPIO_PIN6, GPIO_PIN1, GPIO_PIN17};
static int pressed_recently = 1; // RELEASED

static int  BUTTON_RESET  = 0;
static int  BUTTON_PEACE  = 1;
static int  BUTTON_SHAKA  = 2;
static int  BUTTON_BIRD  = 3;
static int  BUTTON_ROCK =  4;

int mode = PRESET_OFF;

void init_buttons(void) {
    gpio_init();
    for (int i = 0; i < 5; ++i) {
        gpio_set_input(BUTTONS[i]);
    }
}

int preset_mode() {
    unsigned int reset = gpio_read(BUTTONS[0]);
    printf("reset: %d\n", reset);
    if (reset == PUSHED && pressed_recently == RELEASED) {
        printf("Switching preset mode");
        mode = (mode == PRESET_OFF) ? PRESET_ON : PRESET_OFF;
    }
    pressed_recently = reset;
    return mode;
}

void to_preset(void) {
    if (gpio_read(BUTTONS[1]) == PUSHED ) {
        go_to_pose(PEACE);
    } else if (gpio_read(BUTTONS[2]) == PUSHED) {
        go_to_pose(BIRD);
    } else if (gpio_read(BUTTONS[3]) == PUSHED) {
        go_to_pose(ROCK);
    } else {
        go_to_pose(OPEN);
    }
}

