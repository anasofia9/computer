#include "assert.h"
#include "timer.h"
#include "fb.h"
#include "gl.h"
#include "console.h"
#include "printf.h"

/* Note that to use the console, one should only have to
 * call console_init. To use the graphics library, one
 * should only have to call gl_init. If your main() requires
 * more than this, then your code will not pass tests and
 * will likely have many points deducted. Our GL tests
 * will call gl_init then invoke operations; our console
 * tests will call console_init then invoke operations.
 * To guarantee that tests will pass, make sure to also
 * run tests for each component separately.
 */

#define _WIDTH 640
#define _HEIGHT 512

#define _NROWS 10
#define _NCOLS 20

static void test_console(void)
{
    console_init(_NROWS, _NCOLS);
    
//console_putchar('H');
  //  console_putchar('\b');
    //console_putchar('A');
//    for(int i =0; i < 23; i++){ //wrapping doesnt work
//        console_putchar('A');
//    }
    //console_printf("HELLObdhjbdfbrebfjkrebjkrbejkberjkbfuribfeuibfeiubwubeuibeuibwuiwbuiwbuiwebueiwbefuiebuiewbuiewbueibewuiefwbuifewbuiewbuewibewuibewuiebwuiwebuiewbiufbufibfuifbuifbuibewubrguergyergryeg");
   // console_printf("ABCDEFGHIJKLMNOPQDDDAABCDEFGHIJKLMNOPQDDDA");
    //timer_delay(2);
    console_printf("012356789");
    //the CS107e shell. Remember to type on your");
    timer_delay(2);
    console_printf("\f");
    timer_delay(2);
    console_printf("Welcome to the CS107e shell. Remember to type on your");
    timer_delay(2);
    console_printf("\f");
    timer_delay(2);
    console_printf("Welcome to the Ca");
    //Remember to type on your");
   timer_delay(2);
    console_printf("\f");
    timer_delay(2);
//    //CS107E shell. ");
//    timer_delay(5);
//    console_printf("123456789123456789137834783473123456789123456789137834783473123456789123456789137834783473123456789123456789137834783473123456789123456789137834783473123456789123456789137834783473123456789123456789137834783473");
   // console_printf("A");
   // console_printf("12345678912");
//console_printf("Hello my name is Ana\r\n");
//    timer_delay(3);
//    console_printf("\f");
//    timer_delay(2);
//    console_printf("Hi");
//    console_printf("HI\r\n");
}

static void test_gl(void)
{
    gl_init(_WIDTH, _HEIGHT, GL_SINGLEBUFFER);
    
    gl_clear(GL_BLACK);
 //   draw_circle(200 ,200, 50, GL_WHITE);
//    gl_draw_line(155, 228, 155, 500, GL_WHITE);
//    gl_draw_line(245, 228, 245, 500, GL_WHITE);
//    gl_draw_line(155, 173, 110, 100, GL_WHITE);kmjm//    gl_draw_line(245, 173, 200, 100, GL_WHITE);
        draw_palm(320, 370, 160, 120);
    draw_thumb(320, 370, 160, 120);
    draw_index(320, 370, 160, 120);
    draw_middle(320, 370, 160, 120);
    draw_ring(320, 370, 160, 120);
    draw_pinky(320, 370, 160, 120);
}

void main(void)
{
 //test_console();
  test_gl();

    /* TODO: Add tests here to test your graphics library and console.
       For the framebuffer and graphics libraries, make sure to test
       single & double buffering and drawing/writing off the right or
       bottom edge of the frame buffer.
       For the console, make sure to test wrap-around and scrolling.
    */
}
