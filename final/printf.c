#include "printf.h"
#include "uart.h"
#include <stdarg.h>
#include  "strings.h"

#define MAX_OUTPUT_LEN 1024
int power(int a, int e){ //helper function for exponential calculations in unsigned_to_base,
    //a represents base, e represents the exponent
    int b = a;
    for(int i =e-1; i >0; i--){
        b *= a;
    }
    return b;
}
int unsigned_to_base(char *buf, int bufsize, unsigned int val, int base, int min_width)
{
    
    if(bufsize == 0) return 0;
    if(min_width ==0) min_width =1;
    int numdigits = 0;
    // if(val%10) numdigits++; //if val is zero, return empty string or zero?
    int num = val;
    while(num != 0){ //this loop counts the number of digits
        numdigits++;
        num = num/base;
    }
    int num_zeros_needed = 0;
    if(numdigits < min_width){
        num_zeros_needed = min_width - numdigits; //calculates how many zeros are needed
    }
    

    volatile char *next_digit = "0";
    next_digit[1] = '\0';
    int appended = 0;
    for(int i = 0; i < numdigits; i++){
        *next_digit = (val%base) + '0'; //for when you are adding the ones digit
        // + '0' used to convert to character
        if(numdigits - i -1 > 0){ //preventing divide by 0
            *next_digit=  (val/(power(base,(numdigits - i -1))))%base + '0';
        }
        
        if(*next_digit > '9'){ //if *next_digit is supposed to be a hex value that is A-F, this if statement should convert it.
            *next_digit = *next_digit + 39; //adds 39 because there are 39 things in between ascii value of '9' and 'a'.
        }
        memcpy(buf+num_zeros_needed+i, (const char*)next_digit, 1); //using bufsize as upper limit for concatenation
        appended++;
        if(i +1 ==bufsize) break; //prevents not respecting bufsize
        
    }
    //null terminates
    
  //copies data from temp pointer to buf, reserving space for zeros needed.
    
    //null terminates
    for(int i = 0; i < num_zeros_needed; i++){
        buf[i] = '0';
    }
    buf[appended+num_zeros_needed] = '\0'; //null terminate
    if(numdigits > bufsize) buf[bufsize-1] = '\0'; //null terminate at bufsize if necessary
    
    if(bufsize < numdigits) return numdigits;
    if(min_width < numdigits) return numdigits;
    return min_width;
}

int signed_to_base(char *buf, int bufsize, int val, int base, int min_width)
{
    int returnval;
    if(val < 0){
        val *= -1;
        returnval = unsigned_to_base((buf+1), bufsize-1, val, base, min_width-1); //passes in buf+1 to accomodate negative in future. adjusts min_width and bufsize accordingly.
        buf[0] = '-';
        buf[bufsize-1] = '\0';
    }
    else{
        returnval = unsigned_to_base(buf, bufsize, val, base, min_width);
    }
    return returnval;
}

int vsnprintf(char *buf, int bufsize, const char *format, va_list args)
{
    char* curr_format =0;
    int min_width = 0;
    int i;
    int characters_added = 0; //this is a counter to see at what index you are at in buf, so you don't overwrite anything.
    for(i =0; i < strlen(format); i++){
        if(format[i] != '%'){
            memcpy((buf+characters_added), &format[i], 1); //if its just a character.
            characters_added++;
            buf[characters_added] = '\0';
            curr_format = "3"; //setting curr_format to a value that make any if statements below true.
        }
        else{
            if(format[i+1] == 'c') curr_format = "%c"; //figuring out code
            else if(format[i+1] == 's') curr_format = "%s";
            else if(format[i+1] == 'p') curr_format = "%p";
            else if(format[i+1] == 'd') curr_format = "%d";
            else if(format[i+1] == 'x') curr_format = "%x";
            else {
                int j =i+2;
                volatile char* width = "0";
                while(format[j] != 'c' && format[j] != 's' && format[j] != 'p' && format[j] != 'd' && format[j] != 'x'){
                    width[j-i-2] = format[j];
                    j++; //this loop sets the width of a %d or %x argument
                }
                if(format[j] == 'd') curr_format = "&d";
                else if(format[j] == 'x') curr_format = "%x"; //finds format code
                i = j-1;
                char* invalid;
                min_width = strtou((const char*)width, &invalid, 10); //converts width to int
            }
            i++;
        }
        
        volatile int num =0;
        if(strcmp(curr_format, "%d")==0){
            num = va_arg(args, int);
            characters_added += signed_to_base((buf + characters_added), bufsize, num, 10, min_width);
        }
        else if(strcmp(curr_format, "%x")==0){ //how to use this for variable widths?
            num = va_arg(args, int);
            characters_added += unsigned_to_base((buf+ characters_added), bufsize, num, 16, min_width); //should we make unsigned to base return a string with lowercase letters if base is hex?
            //my unsigned to base only uses uppercase letters
            
        }
        else if(strcmp(curr_format, "%p")==0){
            num = va_arg(args, int);
            buf[characters_added] = '0';
            buf[characters_added+1] = 'x'; //puts hex symbols in front of buf
            characters_added += unsigned_to_base((buf+2 +characters_added), bufsize, num, 16, 8); //doesnt work, min width should be eight but padding does not yet work in my function.
            characters_added +=2;
           
        }
        else if(strcmp(curr_format, "%c") ==0){
            volatile char* character = "0";
            
            num = va_arg(args, int);
            *character = (char) num;
            memcpy((buf+ characters_added), (const void*)character, 1);
            characters_added++;
        }
        else if(strcmp(curr_format, "%s")==0){
            char* c = va_arg(args, char*);
            memcpy((buf + characters_added), c, strlen(c)+1);
            characters_added += strlen(c);
        }
    }
    return characters_added;
}

int snprintf(char *buf, int bufsize, const char *format, ...)
{
    char max[MAX_OUTPUT_LEN]; //makes huge buffer
    va_list ap;
    va_start(ap, format);
    int characters_added = vsnprintf(max, MAX_OUTPUT_LEN, format, ap); //call vsnprintf with temporary buffer
    int size_to_pass = bufsize -1;
    if(bufsize > characters_added) size_to_pass = characters_added;
    for(int i =0; i < size_to_pass; i++){
        buf[i] = max[i];
    }//pass necessary info from temporary buffer max to buf
    buf[size_to_pass] = '\0'; //null terminates
    va_end(ap);
    return characters_added;
}

int printf(const char *format, ...)
{
    char max[MAX_OUTPUT_LEN]; //makes huge temporary buffer
    va_list ap;
    va_start(ap, format);
    int n = vsnprintf(max, MAX_OUTPUT_LEN, format, ap);
    va_end(ap);
    for(int i =0; i < n; i++){
        uart_putchar(max[i]); //display char
    }
    return 0;
}
