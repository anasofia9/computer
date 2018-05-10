#include "gpio.h"
#include "timer.h"
void displayDigits(int pin, int digit, int arr[]);
void displayDigitsWithDelay(int pin, int digit, int arr[], int delay);
void turnOnSegments(int digit, int arr[]);
void turnOffSegments();
void main(void)
{
    unsigned int arr[16];
    arr[0] = 0b00111111; //zero digit
    arr[1] = 0b00000110;
    arr[2] = 0b01011011;
    arr[3] = 0b01001111;
    arr[4] = 0b01100110;
    arr[5] = 0b01101101;
    arr[6] = 0b01111101;
    arr[7] = 0b00000111;
    arr[8] = 0b01111111;
    arr[9] = 0b01101111;
    arr[10] = 0b01110111; //digit A
    arr[11] = 0b01111111; //digit B
    arr[12] = 0b00111001; //C
    arr[13] = 0b00111111; //D
    arr[14] = 0b01111001;
    arr[15] = 0b01110001; //

    gpio_set_input(2); //set buttons as inputs
    gpio_set_input(3);

    for(int i = 10; i < 14; i++){ //set all digit pins as outputs
        gpio_set_output(i);
        gpio_write(i, 0);

    }
    for(int i = 20; i < 27; i++){ //set all segments as outputs
        gpio_set_output(i);
        gpio_write(i, 0);
    }


    unsigned int totalSeconds = 0;
    unsigned int seconds = 0;
    unsigned int tenseconds = 0;
    unsigned int minutes = 0;
    unsigned int tenminutes = 0;
    
    unsigned int added_U_Seconds = 0;
    //clock loop
    unsigned int USecondsEnd = 0;
    int startmicro_sec = timer_get_ticks();
    while(1){
        totalSeconds = (USecondsEnd - startmicro_sec)/ 1000000; //change to seconds

        if(totalSeconds > 3600) totalSeconds = totalSeconds%3600; //if more than an hour passed, mods it to start           at 0 again.

        tenminutes = ((totalSeconds)/60)/10;
        displayDigits(10, tenminutes, arr);

        minutes = ((totalSeconds)/60)%10;
        displayDigits(11, minutes, arr);

        tenseconds = (totalSeconds%60)/10;
        displayDigits(12, tenseconds, arr);
        
        seconds = (totalSeconds%60)%10;
        displayDigits(13, seconds, arr);
        
        USecondsEnd = timer_get_ticks();
        
        //Button Extension
        if(gpio_read(2) == 0){
            startmicro_sec -= 60000000; //adds a minute
            timer_delay_us(25000);
            while(gpio_read(2) ==0);
            
        }
        if(gpio_read(3) == 0){
            startmicro_sec -= 10000000; //adds ten seconds
            timer_delay_us(25000);
            while(gpio_read(3) ==0);
        }
        
        
    }
    
    //refresh loop:
//    while(1){
//        displayDigitsWithDelay(10, 1, arr, 2500);
//        displayDigitsWithDelay(11, 2, arr, 2500);
//        displayDigitsWtihDelay(12, 3, arr, 2500);
//        displayDigitsWithDelay(13, 4, arr, 2500);
//    }
}
void displayDigits(int pin, int digit, int arr[]){
    gpio_write(pin, 1); //turns on the digit pin
    turnOnSegments(digit, arr); //turns on desired segments for the digit you want to display
    gpio_write(pin, 0); //turns off the digit pin
    turnOffSegments(); //turns off all segments to that they can be reused
}
void displayDigitsWithDelay(int pin, int digit, int arr[], int delay){
    gpio_write(pin, 1); //turns on digit pin
    turnOnSegments(digit, arr); //turns on desired segments for the digit you want to display
    timer_delay_us(delay); //delay by desired amount
    gpio_write(pin, 0);//turns off the digit pin
    turnOffSegments();//turns off all segments to that they can be reused
}

void turnOnSegments(int digit, int arr[]){
    int bitpattern = arr[digit];
    for(int i =0; i < 8; i++){ //iterate through bits
        int b = bitpattern & 1;
        bitpattern = bitpattern >> 1;
        if(b==1){
            gpio_write(20+ i, 1); //if bit is one, turn this segment on.
        }
    }
    
}

void turnOffSegments(){
    for(int i =20; i < 27; i++){ //turns off all segments
        gpio_write(i, 0);
    }
}
