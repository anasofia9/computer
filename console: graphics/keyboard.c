#include "gpio.h"
#include "gpioextra.h"
#include "keyboard.h"
#include "ps2.h"
#include "printf.h"

const unsigned int CLK  = GPIO_PIN23;
const unsigned int DATA = GPIO_PIN24;
static int modifiers = 0;

void wait_for_falling_clock_edge() {
    while (gpio_read(CLK) == 0) {}
    while (gpio_read(CLK) == 1) {}
}

void keyboard_init(void) 
{
    gpio_set_input(CLK); 
    gpio_set_pullup(CLK); 
 
    gpio_set_input(DATA); 
    gpio_set_pullup(DATA); 
}

unsigned char keyboard_read_scancode(void) 
{
    unsigned char bits =0;
    int ones_counter = 0; //to check if parity is correct
    int parity = 0;
    int valid = 1; //for while loop, valid will change to 0 if stop bit is valid (to stop looping), and change back to 1 to continue loop if parity bit is invalid.
    while(valid){
        for(int i =0; i < 11; i++){
            wait_for_falling_clock_edge();
            int function = gpio_read(DATA);
            if(i > 0 && i < 9){ //if you are looking at a data bit
                if(function ==1) ones_counter++;
                bits += (function << (i-1)); //least significant bit first, so left shift increases for every data bit
            }
            if(i ==10) valid = 0;
            if(i == 10 && function != 1){ //if stop bit is not high, continue looping
                valid = 1;
            }
            if(i ==9) parity = function;
        }
        
        if((ones_counter + parity)%2 !=1) valid = 1; //if the number of ones is not odd, keep looping because parity is wrong.
    }
    return bits;
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
