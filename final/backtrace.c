#include "backtrace.h"
#include "printf.h"

int backtrace(struct frame f[], int max_frames)
{
    //check start function and cur_fp
    int i = 0;
    unsigned int *cur_fp; // unsigned int pointer so that it is 4 bytes because an instruction is four bytes
    __asm__("mov %0, fp" : "=r" (cur_fp));
    while(*(cur_fp - 3) !=0 && i < max_frames){
        struct frame curr_frame;
        curr_frame.resume_addr = (void *)*(cur_fp-1); // resume address is in saved lr, which you have to dereference from address below fp
        unsigned int* start_of_function_ptr = (unsigned int *)*(cur_fp-3);//loaction where saved pc of the previous function is
        unsigned int * start_of_function = (unsigned int*)(*start_of_function_ptr); //saved pc of previous function
        curr_frame.resume_offset = ((unsigned int)curr_frame.resume_addr) - (unsigned int)start_of_function +12; //lr -pc (offset of 12)
        unsigned int name_address = (unsigned int)*(start_of_function-4); //name address is stored in saved pc -4
        int found_length = 0; //variable to check if a valid length was found
        int addr_shifted = name_address >> 24; //upper 8 bits of name address should be 1
        if((addr_shifted & 0xff) == 0xff) found_length =1; //if upper 8 bits are 1, then you found a valid length for the function name
        
        if(!found_length){ //if you didnt find a valid length, put question marks in for name
            char *name = "???";
            curr_frame.name = name;
        }
        else {
            int length_of_name = (name_address & ~0xff000000); //find the actual length of function name
            char* name  = (char*)(start_of_function - length_of_name/4 -4); //-1 because dont want to include size in function name
            curr_frame.name = name;
        }
        
        f[i] = curr_frame;
        i++;
        cur_fp = (unsigned int* )*(cur_fp -3); //have to go to the fp of the other function, which is stored three instructions below cur fp
        //look at saved fp to see the name
    }
    return i;
}

void print_frames (struct frame f[], int n)
{
    for (int i = 0; i < n; i++)
        printf("#%d %p at %s+%d\n", i, f[i].resume_addr, f[i].name, f[i].resume_offset);
}

void print_backtrace (void)
{
    int max = 50;
    struct frame arr[max];

    int n = backtrace(arr, max);
    print_frames(arr+1, n-1);   // print frames starting at this function's caller
}
