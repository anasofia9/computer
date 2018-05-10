#include "shell.h"
#include "shell_commands.h"
#include "keyboard.h"
#include "printf.h"
#include "strings.h"
#include "malloc.h"
#include "pi.h"

#define LINE_LEN 80

static int (*shell_printf)(const char * format, ...);
static int (*shell_putchar)(int ch);
static int num_commands = 5; //used later to know how many commands to iterate through

static const command_t commands[] = {
    {"help",   "<cmd> prints a list of commands or description of cmd", cmd_help},
    {"echo",   "<...> echos the user input to the screen", cmd_echo},
    {"peek",   "prints the contents of memory at hex_addr", cmd_peek},
    {"reboot", "reboots the Raspberry Pi back to the bootloader", cmd_reboot},
    {"poke",   "Stores a value given by user into the memory at an address given by user", cmd_poke},
};

int cmd_echo(int argc, const char *argv[]) 
{
    shell_printf("\n");
    for (int i = 1; i < argc; ++i)
        shell_printf("%s ", argv[i]);
    shell_printf("\n");
    return 0;
}

int cmd_help(int argc, const char *argv[]) 
{
    if(argc==1){ //checks if there are no arguments
        for(int i =0; i < num_commands; i++){ //if no arguments, print all the commands and their descriptions
            shell_printf("\n%s : ", commands[i].name);
            shell_printf("%s" , commands[i].description);
        }
        shell_printf("\n");
        return 0;
    }
    else{
        char* command  = argv[1];
        for(int i =0; i < num_commands; i++){ //iterate to find command description
            if(strcmp(command, commands[i].name)==0){
                shell_printf("\n%s : ", commands[i].name);
                shell_printf("%s\n", commands[i].description);
                return 0;
            }
            
        }
        
        shell_printf("error: no such command '%s\n'", command); //if command not valid
        return 1;
    }
    
    return 0;
}
int cmd_reboot(int argc, const char *argv[]) {
    pi_reboot();
}

int cmd_peek(int argc, const char *argv[]){
    if(argc < 2){
        shell_printf("error: peek expects 1 argument [hex address]");
        return 1;
    }
    char *endptr;
    unsigned int* ptr = strtou(argv[1], &endptr, 16);
    if((unsigned int)(ptr) %4 != 0){ //checks for 4-byte alignment
        shell_printf("error: peek address must be 4-byte aligned");
        return 1;
    }
    if(*endptr != '\0'){ //checks that strtou converted the argument successfully
        shell_printf("error: peek expects 1 argument [hex address]");
        return 1;
    }
    
    shell_printf("\n%p: ", ptr);
    shell_printf("%x", *ptr);
    shell_printf("\n");
    return 0;
}
    
int cmd_poke(int argc, const char *argv[]){
    if(argc <3) shell_printf("error: poke expects 2 arguments [hex address] [hex value]");
    char *endptr;
    unsigned int* ptr = strtou(argv[1], &endptr, 16);
    if((unsigned int)(ptr) %4 != 0){ //checks for 4-byte alignment of given address
        shell_printf("error: poke address must be 4-byte aligned");
        return 1;
    }
    if(*endptr != '\0') { //checks that strtou converted the argument successfully
        shell_printf("error: poke cannot convert %s", argv[1]);
        return 1;
    }
    char *val_endptr;
    unsigned int val = strtou(argv[2], &val_endptr, 16);
    if(*val_endptr != '\0'){
        shell_printf("error: poke cannot convert %s", argv[2]);
        return 1;
    }
    *ptr = val;
    shell_printf("\n%p: %x\n", ptr, *ptr);
    return 0;
}


void shell_init(formatted_fn_t print_fn, char_fn_t putchar_fn)
{
    shell_printf = print_fn;
    shell_putchar = putchar_fn;
}

void shell_bell(void)
{
    shell_putchar('\a');
}

void shell_readline(char buf[], int bufsize)
{
    int copy_bufsize = bufsize;
    int things_added = 0; //functions as an index
    while(bufsize > 1){
        unsigned char character = keyboard_read_next();
        if(character == '\n'){ //if new line, stop adding things
            buf[things_added] = '\0';
            break;
        }
        else if(character == '\b'){ //for backspace
            if(things_added >=1){
                shell_putchar('\b');
                shell_putchar(' ');
                shell_putchar('\b');
                buf[things_added-1] = '\0'; //deletes value that was in buf before beackspace
                things_added--;
                bufsize++;
            }
            else shell_bell();
            
        }
        else{
            shell_putchar(character);
            buf[things_added] = character;
            bufsize--;
            things_added++;
        }
    }
    if(copy_bufsize > things_added) buf[things_added] = '\0';
    else buf[bufsize-1] = '\0';
}

int shell_evaluate(const char *line) //return statement?
{
    char* arr[LINE_LEN];
    int index =0;
    int wordsize = 0;
    int array_index = 0;
    if(line[index] == '\0') return 1;

    while(line[index] != '\0'){
        int index_beginning_of_word  = index;
        wordsize = 0;

        while(line[index] != ' ' && line[index] != '\0'){
            wordsize++; //finds length of word
            index++;
            
        }
        int temp_wordsize = wordsize;
        char word[wordsize+1]; //makes an array for word
        int i =0;
        while(temp_wordsize > 0){
            word[i] = line[index_beginning_of_word+i]; //puts in the characters of the word to the array
            temp_wordsize--;
            i++;
        }
        word[wordsize] = '\0';
        wordsize++; //for the null terminator at the end
        arr[array_index] =  malloc(wordsize); //finds space to put the word in the array
        
        memcpy(arr[array_index], word, wordsize); //copies data from word into the current index of arr
        arr[array_index][wordsize] = '\0'; //null terminates
        array_index++;
    
        if(line[index] == ' ') index++; //if you're at a space, just continue
    }
    char* command = arr[0]; //the command should be the first word, so it should be at index 0
    
    for(int i =0; i < num_commands; i++){ //iterate through commands to see if the word typed first matches any command
        if(strcmp(commands[i].name, command)==0){ //if the word was a valid command, will call its function
            return commands[i].fn(array_index, arr);
           
        }
    }
    free(arr); //frees memory allocated
    return 1;
}

void shell_run(void)
{
    shell_printf("Welcome to the CS107E shell. Remember to type on your PS/2 keyboard!\n");
    while (1) 
    {
        char line[LINE_LEN];
        shell_printf("Pi> ");
        shell_readline(line, sizeof(line));
        shell_evaluate(line);
    }
}
