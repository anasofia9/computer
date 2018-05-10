#include "console.h"
#include "gl.h"
#include "printf.h"
#include "malloc.h"
#include "strings.h"
#define MAX_OUTPUT_LEN 1024

static unsigned int num_rows = 0;
static unsigned int num_cols = 0;
static unsigned int volatile cursor_row = 0;
static unsigned int volatile cursor_column = 0;
static unsigned char *array;
void console_init(unsigned int nrows, unsigned int ncols)
{
    gl_init(ncols * gl_get_char_width(), nrows * gl_get_char_height(), GL_DOUBLEBUFFER); //used to be 640, 512.
    array = malloc(nrows * ncols);
    console_clear();
    num_rows = nrows;
    num_cols = ncols;

}

void console_clear(void)
{
    unsigned char (*array2)[num_cols] = (unsigned char(*) [num_cols])(array);
    for(int i =0; i < num_rows; i++){
        for(int j = 0; j < num_cols; j++){
            array2[i][j] = ' '; //clearing array that keeps track of what is in console. overwrites everything with spaces
        }
    }
    
    gl_clear(GL_BLACK);
    gl_swap_buffer();
    gl_clear(GL_BLACK);
    cursor_row = 0; //sets cursor back at initial position
    cursor_column = 0;
}

void rewrite_buffers(unsigned char (*array2)[num_cols]){
    for(int i =0; i < num_rows; i++){
        for(int j = 0; j < num_cols; j++){ //rewrite everything in array to screen.
            gl_draw_char( j* gl_get_char_width(), i*gl_get_char_height(), array2[i][j], GL_BLUE);
        }
    }
}

void scroll(unsigned char (*array2)[num_cols]){
    for(int i = 0; i+1 < num_rows; i++){
        for(int j = 0; j < num_cols; j++){ //reset array to move every row up by one
            array2[i][j] = array2[i+1][j];
        }
    }
    for(int j = 0; j < num_cols; j++){
        array2[num_rows-1][j] = ' '; //last row is essentially empty, containing only spaces.
    }
    gl_clear(GL_BLACK);
    rewrite_buffers(array2); //had clear black before
    gl_swap_buffer();
    gl_clear(GL_BLACK);
    rewrite_buffers(array2);
    cursor_row--;
    cursor_column =0;
}

void backspace(unsigned char (*array2)[num_cols]){
    if(cursor_column == 0 && cursor_row ==0) return;  //if you're at the first spot, there is nothing to delete, so don't do anything
    if(cursor_column !=0){
        cursor_column--;
    }
    else {
        cursor_column = num_cols-1; //if cursor was at the first column, must go back to previous row.
        cursor_row--;
    }
    gl_draw_rect(cursor_column*gl_get_char_width(), cursor_row*gl_get_char_height(), gl_get_char_width(), gl_get_char_height(), GL_BLACK); //rectangle should cover the character that was there before.
    gl_swap_buffer();
    gl_draw_rect(cursor_column*gl_get_char_width(), cursor_row*gl_get_char_height(), gl_get_char_width(), gl_get_char_height(), GL_BLACK);
    array2[cursor_row][cursor_column] = ' ';
    return;
}

void newline(unsigned char (*array2)[num_cols]){
    for(int i = cursor_column; i < num_cols; i++){ //if there are still spaces in the line cursor is at, write the values of those slots in the array as spaces
        array2[cursor_row][cursor_column] = ' ';
    }
    cursor_column = 0; //reset cursor to first column of next row
    cursor_row++;
    return;
}

int console_putchar(int ch)
{
    unsigned char (*array2)[num_cols] = (unsigned char(*) [num_cols])(array);
    if(cursor_row == num_rows){ //if you're past the number of rows (in the array, cursor row starts at zero so it should go up to num_rows -1), scroll
        scroll(array2);
    }
    if(ch == '\a') return ch; //in case shell bell returns a character, you don't want it to do anything
    else if(ch == '\n'){ //new line handling
        newline(array2);
        return ch;
    }
    else if(ch == '\b'){
        backspace(array2);
        return ch;
    }
    else if(ch == '\f'){
        console_clear();
        return ch;
    }
    else{
        if(cursor_column == num_cols){
            cursor_column = 0;
            cursor_row++;
        }
        gl_draw_char(cursor_column*gl_get_char_width(), cursor_row*gl_get_char_height(), ch, GL_BLUE);
        gl_swap_buffer();
        gl_draw_char(cursor_column*gl_get_char_width(), cursor_row*gl_get_char_height(), ch, GL_BLUE);
        array2[cursor_row][cursor_column] = ch;
        cursor_column++;
    }
	return ch;
}


int console_printf(const char *format, ...)
{
    char buf[MAX_OUTPUT_LEN]; //what length to make this?
    va_list ap;
    va_start(ap, format);
    int num_things_added = vsnprintf(buf, MAX_OUTPUT_LEN, format, ap);
    va_end(ap);
    for(int i = 0; i < num_things_added && i < MAX_OUTPUT_LEN; i++){
        console_putchar(buf[i]); //display char
    }
    
	return 0;
}
