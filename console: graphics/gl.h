#ifndef GL_H
#define GL_H

/*
 * Functions for a simple bare metal Raspberry Pi graphics library
 * that draws pixels, text, lines, triangles, and rectangles. Builds
 * on the lower-level framebuffer library fb.[ch] for framebuffer 
 * access and configuration; trying to use both fb and gl
 * simultaneously is discouraged.
 *
 * You implement this module in assignment 6 (text and rectangles are 
 * required, lines and triangles are an extension).
 *
 * Author: Philip Levis <pal@cs.stanford.edu>
 * Date: Mar 23 2016
 */

#include "fb.h"

enum { GL_SINGLEBUFFER = FB_SINGLEBUFFER, GL_DOUBLEBUFFER = FB_DOUBLEBUFFER };

/*
 * Initialize the graphic library. This function will call fb_init in turn 
 * to initialize the framebuffer. The framebuffer will be initialzed to
 * 4-byte depth (32 bits per pixel).
 *
 * @param width  the requested width in pixels of the framebuffer
 * @param height the requested height in pixels of the framebuffer
 * @param mode   whether the framebuffer should be
 *                      single buffered (GL_SINGLEBUFFER)
 *                      or double buffered (GL_DOUBLEBUFFER)
 */
void gl_init(unsigned int width, unsigned int height, unsigned int mode);

/*
 * Get the current width in pixels of the framebuffer.
 *
 * @return    the width in pixels
 */
unsigned int gl_get_width(void);

/*
 * Get the current height in pixels of the framebuffer.
 *
 * @return    the height in pixels
 */
unsigned int gl_get_height(void);

/*
 * Define a type for color. We use BGRA colors, where each color
 * component R, B, G, or A is a single unsigned byte. The least
 * signficant byte is the B component, and A is most significant.
 */
typedef unsigned int color;

/*
 * Define some common colors ...
 *
 * Note that colors are BGRA, where B is the first byte in memory
 * and the least significant byte in the unsigned word.
 */
#define GL_BLACK   0xFF000000
#define GL_WHITE   0xFFFFFFFF
#define GL_RED     0xFFFF0000
#define GL_GREEN   0xFF00FF00
#define GL_BLUE    0xFF0000FF
#define GL_CYAN    0xFF00FFFF
#define GL_MAGENTA 0xFFFF00FF
#define GL_YELLOW  0xFFFFFF00
#define GL_AMBER   0xFFFFBF00

/*
 * Returns a color composed of the specified red, green, and
 * blue components. The alpha component of the color will be
 * set to 0xff (fully opaque).
 *
 * @param r  the red component of the color
 * @param g  the green component of the color
 * @param b  the blue component of the color
 *
 * @return   the color as a single value of type color.
 */
color gl_color(unsigned char r, unsigned char g, unsigned char b);

/*
 * Clear all the pixels in the framebuffer to the given color.
 *
 * @param c  the color drawn into the framebuffer
 */
void gl_clear(color c);

/*
 * Swap the front and back buffers. The draw buffer is moved to the 
 * front (displayed) and the front buffer is moved to the back
 * (becomes the draw buffer).
 *
 * If not in double-buffer mode, this function has no effect.
 */
void gl_swap_buffer(void);

/*
 * Draw a single pixel in color `c`. If the pixel location is
 * outside the bounds of framebuffer, it is not drawn. 
 *
 * @param x  the x location of the pixel.
 * @param y  the y location of the pixel.
 * @param c  the color c of the pixel
 */
void gl_draw_pixel(int x, int y, color c);

/*
 * Read a single pixel.  If the pixel location is outside the bounds
 * of the framebuffer, zero is returned.
 *
 * @param x  the x location of the pixel.
 * @param y  the y location of the pixel.
 *
 * @return    the color at that location.
 */
color gl_read_pixel(int x, int y);

/*
 * The following functions draw characters and strings.
 */

/*
 * Draw a single character. This function only draws pixels that
 * are inside the framebuffer. The character is clipped to the
 * left, right, top and bottom sides of the framebuffer.
 *
 * @param x  the x location of the top left corner of the character glyph.
 * @param y  the y location of the top left corner of the character glyph.
 * @param ch  the character to be drawn, e.g. 'a'. Only characters
 *                with ASCII values between '!' and '~' are drawn.
 * @param c  the color of the character
 */
void gl_draw_char(int x, int y, int ch, color c);

/*
 * Draw a string.
 *
 * @param x  the x location of the top left corner of the string.
 * @param y  the y location of the top left corner of the string.
 * @param string  the null-terminated string to be drawn. Characters
 *                are drawn left to right.
 * @param c  the color c of the character
 */
void gl_draw_string(int x, int y, char* string, color c);


/*
 * Get the height in pixels of a single character glyph.
 *
 * @return the character height in pixels
 */
unsigned int gl_get_char_height(void);

/*
 * Get the width in pixels of a single character glyph.
 *
 * @return the character width in pixels
 */
unsigned int gl_get_char_width(void);


/*
 * Draw a filled rectangle. All pixels inside the rectangle are drawn.
 * The rectangle is clipped to the left, right, top and bottom sides 
 * of the framebuffer.
 *
 * @param x  the x location of the upper left corner of the rectangle
 * @param y  the y location of the upper left corner of the rectangle
 * @param w  the width of the rectangle
 * @param h  the height of the rectangle
 * @param c  the color c of the rectangle
 */
void gl_draw_rect(int x, int y, int w, int h, color c);

/*
 * Draw a line segment. The line is clipped to the left, right, 
 * top and bottom sides of the framebuffer.
 *
 * @param x1  the x location of vertex 1
 * @param y1  the y location of vertex 1
 * @param x2  the x location of vertex 2
 * @param y2  the y location of vertex 2
 * @param c   the color c of the line
 *
 * This function is not part of the basic requirements.
 * You can leave this function unimplemented if you are 
 * not doing the extension.
 */
void gl_draw_line(int x1, int y1, int x2, int y2, color c);

/*
 * Draw a filled triangle. All pixels inside the triangle are drawn.
 * The triangle is clipped to the left, right, top and bottom sides 
 * of the framebuffer.
 *
 * @param x1  the x location of vertex 1
 * @param y1  the y location of vertex 1
 * @param x2  the x location of vertex 2
 * @param y2  the y location of vertex 2
 * @param x3  the x location of vertex 3
 * @param y3  the y location of vertex 3
 * @param c   the color c of the triangle
 *
 * This function is not part of the basic requirements.
 * You can leave this function unimplemented if you are 
 * not doing the extension.
 */
void gl_draw_triangle(int x1, int y1, int x2, int y2, int x3, int y3, color c);
void draw_circle(int x1, int y1, double radius, color c);
#endif
