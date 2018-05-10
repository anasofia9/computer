#include "gl.h"
#include "fb.h"
#include "font.h"
#include "strings.h"
#include "timer.h"

void gl_init(unsigned int width, unsigned int height, unsigned int mode)
{
    fb_init(width, height, 4, mode);
}

void gl_swap_buffer(void)
{
    fb_swap_buffer();
}

unsigned int gl_get_width(void) 
{
    return fb_get_width();
}

unsigned int gl_get_height(void)
{
    return fb_get_height();
}

color gl_color(unsigned char r, unsigned char g, unsigned char b)
{
    color c = GL_BLACK; //black is 0xff000000, which has only the alpha component. Start with this to always have alpha
    c = c | b;
    c = c | (g << 8);
    c = c | (r << 16);
    return c;
}

void gl_clear(color c)
{
    for(int i =0; i < gl_get_height(); i++){
        for(int j =0; j < gl_get_width(); j++){
            gl_draw_pixel(j, i, c);
        }
    }
}

void gl_draw_pixel(int x, int y, color c)
{
    if(y > gl_get_height() || x > gl_get_width()) return; //check if in bounds of framebuffer
    unsigned (*fb)[fb_get_pitch() /4] = (unsigned (*)[fb_get_pitch() /4])fb_get_draw_buffer(); //using pitch because GPU may make rows a little wider for alignment reasons
    fb[y][x] = c;
}

color gl_read_pixel(int x, int y)
{
    if(y > gl_get_height() || x > gl_get_width()) return 0;
    unsigned (*fb)[fb_get_pitch() /4] = (unsigned (*)[fb_get_pitch() /4])fb_get_draw_buffer(); //using pitch for same reason as above.
    return fb[y][x];
}

void gl_draw_rect(int x, int y, int w, int h, color c)
{
    for(int i = y;  i-y < h; i++){
        for(int j =x; j - x < w; j++){
            gl_draw_pixel(j, i, c);
        }
    }
}

void gl_draw_char(int x, int y, int ch, color c)
{
    if(ch < '!' || ch > '~') return; //only draw printable characters, these are bounds
    unsigned char buf[gl_get_char_height()*gl_get_char_width()];
    font_get_char(ch,buf , font_get_size());
    unsigned char (*fb)[gl_get_char_width()]  = (unsigned char (*)[gl_get_char_width()])buf;
    for(int i = 0; i < gl_get_char_height(); i++){
        for(int j =0; j < gl_get_char_width(); j++){
            if(fb[i][j] ==0xff) gl_draw_pixel(j+x, i+y,c); //0xff represents if that pixel of the character is supposed to be 'on' or colored
        }
    }
}

void gl_draw_string(int x, int y, char* str, color c)
{
    for(int i =0; i < strlen(str); i++){
        if(x + i*gl_get_char_width() > gl_get_width() - gl_get_char_width()){ //check if going past width to see if you need to go to next line
            gl_draw_char(gl_get_char_width(), y+1 , str[i], c);
            x = 0;
            y++;
        }
        
        else{
            gl_draw_char(x + i* gl_get_char_width(), y , str[i], c);
        }
    }
}

unsigned int gl_get_char_height(void)
{
    return font_get_height();
}

unsigned int gl_get_char_width(void)
{
    return font_get_width();
}
//the draw line extension was implemented using Xiaolin Wu's line algorithm https://en.wikipedia.org/wiki/Xiaolin_Wu%27s_line_algorithm
int floor(double x){ //returns the nearest integer value which is less than or equal to the argument of the function
    if(x>0) return (int)x;
    return (int)(x - 0.99999999999999);
}
int ipart(double x){ //integer part of x
    return floor(x);
}
int round(double x){
    return ipart(x +0.5);
}
double fpart(double x){ //fractional part of x (after decimal point).
    return x - floor(x);
}
double rfpart(double x){
    return 1 -fpart(x);
}
int absolute_value(int arg){
    if(arg < 0) return arg*=-1;
    return arg;
}
double find_slope(int x1, int x2, int y1, int y2){
    double dx = x2 - x1;
    double dy = y2 - y1;
    double slope =1;
    if(dx != 0){
        slope = dy/dx;
    }
    return slope;
}
color get_color_with_brightness(color c, double brightness){
    double b = c & 0xff;
    double g = (c >> 8) & 0xff; //255; equivalent of binary 11111111
    double r = (c >> 16) & 0xff; //255;
    b = b * brightness;
    g = g * brightness;
    r = r * brightness;
    int b_int = round(b);
    int g_int = round(g);
    int r_int = round(r);
    return gl_color(r_int, g_int, b_int);
}

void steep_endpoint_draw(int x1, int y1, double slope, color c){
    int xend = round(x1); //first endpoint handling
    double yend = y1 + slope * (xend -x1);
    int xgap = rfpart(x1 +0.5);
    int xpx11 = xend;
    int ypx11 = ipart(yend);
    gl_draw_pixel(ypx11, xpx11, get_color_with_brightness(c, rfpart(yend) * xgap));
    gl_draw_pixel(ypx11+1, xpx11, get_color_with_brightness(c, fpart(yend)* xgap));
}

void non_steep_endpt_draw(int x1, int y1, double slope, color c){
    int xend = round(x1); //first endpoint handling
    double yend = y1 + slope * (xend -x1);
    int xgap = rfpart(x1 +0.5);
    int xpx11 = xend;
    int ypx11 = ipart(yend);
    gl_draw_pixel(xpx11, ypx11, get_color_with_brightness(c, rfpart(yend) * xgap));
    gl_draw_pixel(xpx11, ypx11+1, get_color_with_brightness(c, fpart(yend)* xgap));
}


void gl_draw_line(int x1, int y1, int x2, int y2, color c){
    int a = absolute_value(y2 - y1);
    int b = absolute_value(x2 -x1);
    int steep = 0;
    if(a > b) steep = 1;
    if(steep){
        int tempx = x1; //swapping y's for x's
        x1 = y1;
        y1 = tempx;
        int tempx2 = x2;
        x2 = y2;
        y2 = tempx2;
    }
    if(x1 > x2){ //swapping x1 for x2 and y1 for y2
        int tempx = x1;
        x1 = x2;
        x2 = tempx;
        int tempy = y1;
        y1 = y2;
        y2 = tempy;
        
    }
    double slope = find_slope(x1, x2, y1, y2);
    if(steep) steep_endpoint_draw(x1, y1, slope, c);
    else{
        non_steep_endpt_draw(x1, y1, slope, c);
    }
    if(steep) steep_endpoint_draw(x2, y2, slope, c);
    else{
        non_steep_endpt_draw(x2, y2, slope, c);
    }
    int xend = round(x1);
    double yend = y1 + slope * (xend -x1);
    double inter_y = yend + slope;
    for(int x = xend +1; x <= round(x2) -1; x++){
        if(steep){
            gl_draw_pixel(ipart(inter_y), x, get_color_with_brightness(c, rfpart(inter_y)));
            gl_draw_pixel(ipart(inter_y)+1, x, get_color_with_brightness(c, fpart(inter_y)));
        }
        else{
            gl_draw_pixel(x, ipart(inter_y), get_color_with_brightness(c, rfpart(inter_y)));
            gl_draw_pixel(x, ipart(inter_y)+1, get_color_with_brightness(c, fpart(inter_y)));
        }
        inter_y = inter_y + slope;
    }
    
}
int get_max(int a, int b, int c){
    int max = a;
    if(b > max) max =b;
    if(c > max) max =c;
    return max;
}
int get_min(int a, int b, int c){
    int min = a;
    if(b < min) min =b;
    if(c < min) min =c;
    return min;
}
//using algorithm to see if point is inside triangle found on http://2000clicks.com/mathhelp/GeometryPointAndTriangle2.aspx

void gl_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, color c){
    gl_draw_line(x1, y1, x2, y2, c); //line from vertex 1 to vertex 2
    gl_draw_line(x2, y2, x3, y3, c); //line from vertex 2 to vertex 3
    gl_draw_line(x1, y1, x3, y3, c); //line from vertex 1 to vertex 3 //this line is wrong
   
    int square_x_mincoord = get_min(x1, x2, x3);
    int square_x_maxcoord = get_max(x1, x2, x3);
    int square_y_mincoord = get_min(y1, y2, y3);
    int square_y_maxcoord = get_max(y1, y2, y3);
    
    //found min and max coordinates to make a kind of rectangular region with encompasses triangle so that you don't have to iterate through every pixel for efficiency.
    for(int y = square_y_mincoord+1; y <= square_y_maxcoord; y++){
        for(int x = square_x_mincoord+1; x <= square_x_maxcoord; x++){
            double ab = (y-y1)*(x2-x1) - (x-x1)*(y2-y1);
            double ca = (y-y3)*(x1-x3) - (x-x3)*(y1-y3);
            double bc = (y-y2)*(x3-x2) - (x-x2)*(y3-y2);
            if(ab * bc > 0 && bc *ca >0){
                gl_draw_pixel(x, y, c);
            }

    }
}
}


