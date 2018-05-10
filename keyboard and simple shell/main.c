#include "uart.h"
#include "keyboard.h"
#include "printf.h"
#include "shell.h"


void main(void) 
{
    uart_init();
    keyboard_init();
    shell_init(printf, uart_putchar);

    shell_run();
}
