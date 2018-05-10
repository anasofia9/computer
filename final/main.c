#include "keyboard.h"
#include "console.h"
#include "shell.h"
#include "interrupts.h"
#include "assert.h"
#include "ringbuffer.h"
#include "uart.h"

#define NROWS 20
#define NCOLS 40
extern int cnt;
extern unsigned char _bits;
void main(void) 
{
    gpio_init();
    uart_init();
    keyboard_init();
    console_init(NROWS, NCOLS);
    shell_init(console_printf, console_putchar);
    
    interrupts_global_enable(); // everything fully initialized, now turn on interrupts
   // shell_init(printf, uart_putchar);
    shell_run();
    
//    int oldcnt = 0;
//    while (1) {
//        if(rb_dequeue(rb, num)){
//            if(last != *num){
//                printf("count: %d\n", last);
//
//            }
//        }
//    }
    int oldcnt = 0;
    while (1) {
        if(oldcnt != cnt){
            oldcnt = cnt;
            if(oldcnt%11 == 0){
                printf("%d\n" , _bits);
            }
        }
    }
    
}
