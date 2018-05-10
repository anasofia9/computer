#include "gpio.h"
#include "gpioextra.h"
#include "interrupts.h"
#include "ringbuffer.h"
#include "keyboard.h"
#include "ps2.h"
#include "printf.h"
#include "timer.h"
#include "assert.h"
#include <stdbool.h>

const unsigned int CLK  = GPIO_PIN23;
const unsigned int DATA = GPIO_PIN24;
static int modifiers = 0;
volatile int num_bit = 0;
volatile int cnt = 0;
 rb_t * rb;
volatile unsigned char _bits =0;
 int ones_counter = 0;

void wait_for_falling_clock_edge() {
    while (gpio_read(CLK) == 0) {}
    while (gpio_read(CLK) == 1) {}
}

static void keyboard_handler(unsigned int pc){
   
    if(gpio_check_and_clear_event(CLK)){
        cnt++;
        int function = gpio_read(DATA);
        if(num_bit == 0 && function ==0){ //start bit must be low
             num_bit++;
            _bits = 0;
            ones_counter = 0;
        }
        
        else if(num_bit > 0 && num_bit < 9){
            if(function ==1) ones_counter++;
            _bits += (function << (num_bit-1)); //least significant bit first
            num_bit++;
        }
        else if(num_bit ==9){ //parity must make the number of ones odd
            if((ones_counter + function)%2 ==1){
                num_bit++;
            }
            else if((ones_counter + function)%2 !=1){ //if number of ones not odd, invalid input
                num_bit = 0;
                _bits =0;
            }
        }
        else if(num_bit == 10){
            if(function) rb_enqueue(rb, _bits); //stop bit must be high
            num_bit = 0;
        }
        
        
    }
    
}

void keyboard_init(void) 
{
    gpio_set_input(CLK); 
    gpio_set_pullup(CLK); 
 
    gpio_set_input(DATA); 
    gpio_set_pullup(DATA);
   
    gpio_enable_event_detection(CLK, GPIO_DETECT_FALLING_EDGE);
    bool ok = interrupts_attach_handler(keyboard_handler);
    assert(ok);
    interrupts_enable_source(INTERRUPTS_GPIO3);
    rb = rb_new();

}

unsigned char keyboard_read_scancode(void)
{
    int p;
    while(1){
        if(!rb_empty(rb)){
            if(rb_dequeue(rb, &p))
                return (unsigned char) p;
        }
        timer_delay_ms(2);
    }
    
    
}

int keyboard_read_sequence(unsigned char seq[])
{
    int length = 0;
    seq[0] = keyboard_read_scancode();
    length++;
    if(seq[0] == 0xF0 ){ //0xF0 is for release, which is always followed by another scancode
        seq[1] = keyboard_read_scancode();
        length++;
    }
    else if(seq[0] == 0xE0){ //0xE0 can be followed by up to two things
        seq[1] = keyboard_read_scancode();
        length++;
        if(seq[1] == 0xF0){ //if 0xF0 follows 0xE0, there is still another scancode to look at.
            seq[2] = keyboard_read_scancode();
            length++;
        }
    }
    
    return length;
}
//modifiers use bit field
//update global state in keyboard read event
//gobal variable keeps track of what it is
//event tells you what it was at time of event
// read event is giving you actions
//read next calls read event
//if key goes back up clear the modifier
//caps lock special case
key_event_t keyboard_read_event(void)
{
    key_event_t event;
    unsigned char seq[3];
    ps2_key_t key_temp;
    int action = KEYBOARD_ACTION_DOWN; //will change if key was released. If 0xF0 (release scancode) not present, action is always down.
    event.seq_len = keyboard_read_sequence(seq);
    for(int i =0; i < event.seq_len; i++){
        event.seq[i] = seq[i];
        
        if(seq[i] == 0xF0) action = KEYBOARD_ACTION_UP;
        if(i == event.seq_len-1){ //only the last scancode in seq[] can be a modifier or character
            key_temp = ps2_keys[seq[i]];
            if(key_temp.ch == PS2_KEY_ALT){ // alt
                modifiers = modifiers | KEYBOARD_MOD_ALT;
                if(action == KEYBOARD_ACTION_UP) modifiers = modifiers ^ KEYBOARD_MOD_ALT; //exclusive or
            }
            else if(key_temp.ch == PS2_KEY_SHIFT){ //shift
                modifiers = modifiers | KEYBOARD_MOD_SHIFT;
                if(action == KEYBOARD_ACTION_UP) modifiers = modifiers ^ KEYBOARD_MOD_SHIFT;
            }
            else if(key_temp.ch == PS2_KEY_CTRL){ //cntrl

                modifiers = modifiers | KEYBOARD_MOD_CTRL;
                if(action == KEYBOARD_ACTION_UP) modifiers = modifiers ^ KEYBOARD_MOD_CTRL;
            }
            else if(key_temp.ch == PS2_KEY_CAPS_LOCK && action == KEYBOARD_ACTION_DOWN){ //only changes if keyboard action down, release of caps lock doesn't do anything.
                modifiers = modifiers ^ KEYBOARD_MOD_CAPS_LOCK;
           
            }
            else if(key_temp.ch == PS2_KEY_NUM_LOCK){ //num lock
                modifiers = modifiers | KEYBOARD_MOD_NUM_LOCK;
                if(action == KEYBOARD_ACTION_UP) modifiers = modifiers ^ KEYBOARD_MOD_NUM_LOCK;
            }
            else if(key_temp.ch == PS2_KEY_SCROLL_LOCK){ //scroll lock
                modifiers = modifiers | KEYBOARD_MOD_SCROLL_LOCK;
                if(action == KEYBOARD_ACTION_UP) modifiers = modifiers ^ KEYBOARD_MOD_SCROLL_LOCK;
            }
        }
       
    } //update event
    event.action = action;
    event.modifiers = modifiers;
    event.key = key_temp;
    return event;
}


unsigned char keyboard_read_next(void)
{
    key_event_t curr_event = keyboard_read_event();
    //while loop keeps reading events until you get an event whose action is KEYBOARD_ACTION_DOWN and is not a modifier. This is because you don't want to print anything for shift or caps lock, you only want those keys to modify letters or numbers.
    while(!(curr_event.action == KEYBOARD_ACTION_DOWN && curr_event.key.ch < PS2_KEY_SHIFT)){
        curr_event = keyboard_read_event();
    }
   
    unsigned char ascii_character = curr_event.key.ch;
    int character = ascii_character - '0';
    if(curr_event.action == KEYBOARD_ACTION_DOWN){
        if(character <= 0x7f){ //check if it is a printable crharacte
            if((curr_event.modifiers & 1 <<2) == KEYBOARD_MOD_CAPS_LOCK){ //caps lock doesn't change the number keys along with `, -, and =
                if((ascii_character < 48 || ascii_character > 57) && ascii_character!= 96 &&
                        ascii_character != 45 && ascii_character != 61)//check if not number or = - `; make constant later
                            ascii_character = curr_event.key.other_ch; //only capitalizes if character is a letter
            }
            if((curr_event.modifiers & 1 << 3) == KEYBOARD_MOD_SHIFT) ascii_character = curr_event.key.other_ch; //shift changes numbers as well, so no checks needed.
        }
        
        return ascii_character;
        
    }
}
