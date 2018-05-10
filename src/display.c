#include <math.h>
//
//  Created by Ana Sofia Nicholls on 3/14/18.
//

#include "display.h"
#include "glove.h"

//cannot make static const variables
const int FRONT_PALM_X = 100; //center coordinate for front view
const int SIDE_PALM_X = 450; //center coordinate for side view
const int PALM_Y = 370;
const int FRONT_PALM_WIDTH = 150;
const int PALM_HEIGHT = 160;
const int FINGER_WIDTH = 26;
const int PALM_COLOR = GL_WHITE;
const int FINGERS_Y_OFFSET[5] = {80, 7, 0, 0, 6}; //the fingers around sides of palm in front view must be a little lower because of the rounded edges, this offset will be used to account for that
const int FIRST_SEGMENT_lengths[5] = {50, 60, 70, 60, 50};
const int SECOND_SEGMENT_lengths = 45;
const int THIRD_SEGMENT_lengths = 30;
const int FINGER_COLORS[5] = {GL_RED, GL_BLUE, GL_AMBER, GL_MAGENTA, GL_CYAN};
const int THUMB_INITIAL_ANGLE = 45;
const double CONVERSION_DEGREE_TO_RADIAN = 0.0174533;
const int IS_THUMB = 1;
static struct FingerDisplay {
    color color;
    unsigned int joints_front_x[4]; //these arrays represent coordinates of the circles which represent joints
    unsigned int joints_front_y[4];
    unsigned int joints_side_x[4];
    unsigned int joints_side_y[4];
    unsigned int lengths[3];
    double angle;
};

static struct FingerDisplay fingers[5];

enum {
    FRONT = 0,
    SIDE = 1,
};

void update_display();
static void draw_finger(struct FingerDisplay* finger, unsigned int view, int is_thumb);
static void draw_segment(int x1, int y1, int x2, int y2, color c);
static void draw_palm();
static void draw_side_view();
static void draw_side_palm();
static void draw_front_view();
static void clear_hands();
static void calculate_segment_pos_thumb(struct FingerDisplay *f);
static void calculate_segment_pos_digits(struct FingerDisplay *f);
static void initialize_thumb();

void init_display(){
    gl_init(640, 512, GL_DOUBLEBUFFER);
    gl_clear(GL_BLACK);
    //initializing thumb data
    fingers[0].joints_front_x[0] = FRONT_PALM_X + FRONT_PALM_WIDTH/2;
    fingers[0].joints_front_y[0] = PALM_Y - PALM_HEIGHT/4;
    fingers[0].joints_side_x[0] = SIDE_PALM_X;
    fingers[0].joints_side_y[0] = PALM_Y - PALM_HEIGHT/4;
    fingers[0].lengths[0] = FIRST_SEGMENT_lengths[0]; //thumb only has two segments, so use only two lengths
    fingers[0].lengths[1] = THIRD_SEGMENT_lengths;
    fingers[0].color = FINGER_COLORS[0];
    initialize_thumb(); //must initialize the other joints
    for (finger f = POINTER; f <= PINKY; ++f) {
        fingers[f].color = FINGER_COLORS[f];
        for(int i = 0; i < 4; i++){
            if(f == POINTER || f == MIDDLE){
                fingers[f].joints_front_x[i] = FRONT_PALM_X + FRONT_PALM_WIDTH/(f*2) - 20;
            }
            else{
                fingers[f].joints_front_x[i] = FRONT_PALM_X - FRONT_PALM_WIDTH/(2*(5-f)) +20;
            }
        }
        
        fingers[f].joints_side_x[0] = SIDE_PALM_X;
        fingers[f].joints_front_y[0] = PALM_Y - PALM_HEIGHT/2 + FINGERS_Y_OFFSET[f];
        fingers[f].joints_side_y[0] = fingers[f].joints_front_y[0];
        fingers[f].lengths[0] = FIRST_SEGMENT_lengths[0];
        fingers[f].lengths[1] = SECOND_SEGMENT_lengths;
        fingers[f].lengths[2] = THIRD_SEGMENT_lengths;
    }
}
/* The y values of the thumb never change, so this function initializes them.
 */
static void initialize_thumb(){
    int x_coord_center = fingers[0].joints_front_x[0];
    int y_coord_center = fingers[0].joints_front_y[0];

    double slope = 0.5;
    double length = 70;
    
    int x1 = x_coord_center + FINGER_WIDTH/2;
    int y1 = y_coord_center - FINGER_WIDTH/4;
    int x2 = x1 + (slope * length);
    int y2 = y1 - (slope * length);

    fingers[0].joints_front_x[1] = x2 - FINGER_WIDTH/2;
    fingers[0].joints_front_y[1] = y2 + FINGER_WIDTH/4;
    fingers[0].joints_side_y[1] = y2 + FINGER_WIDTH/4;
   
    double length2 = 40;
    int x3 = x2 + (slope * length2);
    int y3 = y2 - (slope * length2);
    y3 = (y2 +10) - (slope * length2);
    fingers[0].joints_front_x[2] = x3 - FINGER_WIDTH/2 + 3;
    fingers[0].joints_front_y[2] = y3 - FINGER_WIDTH/13;
    fingers[0].joints_side_y[2] = y3 - FINGER_WIDTH/13;
}
//This function calculates the coordinates of each joint for a finger
static void calculate_segment_pos_digits(struct FingerDisplay *f) {
    for(int i = 1; i <= 3; i++){
        int y_displacement_segment = cos((double)f->angle * i)*f->lengths[i-1];
        if( i ==3) y_displacement_segment = fabs(cos((double)f->angle * i)*f->lengths[i-1]);
        if(f->angle == 90 * CONVERSION_DEGREE_TO_RADIAN && i == 3) y_displacement_segment = fabs(cos((double)f->angle/3)* f->lengths[i-1]); //if angle is 90, hand must curl so angle must be smaller
        
        f->joints_front_y[i] = f->joints_front_y[i-1] - y_displacement_segment;
        f->joints_side_y[i] = f->joints_side_y[i-1] - y_displacement_segment;
        if(i == 3 && f->angle > 35 * CONVERSION_DEGREE_TO_RADIAN)  f->joints_side_y[i] = f->joints_side_y[i-1] + y_displacement_segment;

        int x_displacement_segment = sin((double)f->angle * i)*f->lengths[i-1];
        f->joints_side_x[i] = f->joints_side_x[i-1] - x_displacement_segment;
    }
}
//This function calculates the corrdinates of each joint for the thumb
static void calculate_segment_pos_thumb(struct FingerDisplay *f){
    double angle  = f->angle;
    if(f->angle > (90 * CONVERSION_DEGREE_TO_RADIAN)){
        angle = f->angle - (90 * CONVERSION_DEGREE_TO_RADIAN);
        for(int i = 1; i <= 2; i++){
            int x_displacement_segment = sin((double)angle)*f->lengths[i-1];
            f->joints_side_x[i] = f->joints_side_x[i-1] - x_displacement_segment;
            f->joints_front_x[i] = f->joints_front_x[i-1] - x_displacement_segment;
        }
        return;
    }
    else{
        for(int i = 1; i <= 2; i++){
            int x_displacement_segment = cos((double)angle)*f->lengths[i-1];
            f->joints_side_x[i] = f->joints_side_x[i-1] - x_displacement_segment;
            f->joints_front_x[i] = f->joints_front_x[i-1] + x_displacement_segment;
        }
    }
}
//Clears the hand so that its previous state is no longer on the screen
static void clear_hands() {
    unsigned (*fb)[fb_get_pitch() /4] = (unsigned (*)[fb_get_pitch() /4])fb_get_draw_buffer();
    for (int y = PALM_Y - PALM_HEIGHT/2 - 150; y <= PALM_Y + PALM_HEIGHT/2 ; y++){
        for (int x = FRONT_PALM_X - FRONT_PALM_WIDTH/2 - 50; x <= FRONT_PALM_X + FRONT_PALM_WIDTH/2 + 150; x++){
            fb[y][x] = GL_BLACK;
            fb[y][x+250] = GL_BLACK; //this offset is to clear the sideview of the hand.
        }
    }
}

void update_display() {
    fingers[0].angle = (angle(THUMB) + THUMB_INITIAL_ANGLE) * CONVERSION_DEGREE_TO_RADIAN;

    calculate_segment_pos_thumb(&fingers[0]);

    for (finger f = POINTER; f <= PINKY; ++f) {
            // The angle(finger) method comes from glove.h
        fingers[f].angle = (angle(f) * CONVERSION_DEGREE_TO_RADIAN);
        calculate_segment_pos_digits(&fingers[f]);
    }
    clear_hands();
    draw_front_view();
    draw_side_view();
    
    gl_swap_buffer();
    clear_hands();
    draw_front_view();
    draw_side_view();
    
}

static void draw_front_view(){
    draw_palm();
    for (int f = POINTER; f <= PINKY; ++f) {
            draw_finger(&fingers[f], FRONT, 0);
    }
    draw_finger(&fingers[0], FRONT, IS_THUMB); //want thumb drawn last so it is on top.
}

static void draw_side_palm() {
    draw_filled_rect(SIDE_PALM_X, PALM_Y, FINGER_WIDTH/2, (PALM_HEIGHT - FINGER_WIDTH)/2, PALM_COLOR);
    draw_filled_circle(SIDE_PALM_X, PALM_Y - (PALM_HEIGHT - FINGER_WIDTH)/2, FINGER_WIDTH/2, PALM_COLOR);
    draw_filled_circle(SIDE_PALM_X, PALM_Y + (PALM_HEIGHT - FINGER_WIDTH)/2, FINGER_WIDTH/2, PALM_COLOR);
}

static void draw_side_view(){
    draw_side_palm();
    for (int f = PINKY; f >= THUMB; --f) {
        if(f == THUMB) draw_finger(&fingers[f], SIDE, IS_THUMB);
        else{
            draw_finger(&fingers[f], SIDE, 0);
        }
    }
}

static void draw_palm(){
    int square_height = (3*PALM_HEIGHT)/4;
    draw_filled_rect(FRONT_PALM_X, PALM_Y, FRONT_PALM_WIDTH/2, square_height/2, PALM_COLOR);
    int rect_half_width = (3*PALM_HEIGHT)/8;
    int rect_half_height = PALM_HEIGHT/16;
    
    draw_filled_rect(FRONT_PALM_X, PALM_Y + square_height/2 + rect_half_height, rect_half_width , rect_half_height, PALM_COLOR);
    draw_filled_rect(FRONT_PALM_X, PALM_Y - square_height/2 - rect_half_height, rect_half_width, rect_half_height, PALM_COLOR);
    
    int x_coords_circle[2] = {FRONT_PALM_X - FRONT_PALM_WIDTH/2 + FRONT_PALM_WIDTH/8, FRONT_PALM_X + FRONT_PALM_WIDTH/2 - FRONT_PALM_WIDTH/8};
    int y_coords_circle[2] = {PALM_Y - (3*PALM_HEIGHT)/8, PALM_Y + (3*PALM_HEIGHT)/8};
    
    for(int x = 0; x < 2; x++){
        for(int y = 0; y < 2; y++){
            draw_filled_circle(x_coords_circle[x], y_coords_circle[y], PALM_HEIGHT/8, PALM_COLOR);
        }
    }
}

static int abs_substraction(int x1, int x2){
    int sub = x2 - x1;
    if(x2 < x1) sub = x1 - x2;
    return sub;
}
//This function draws a segment. The two points in the parameters represent two joints.
static void draw_segment(int x1, int y1, int x2, int y2, color c){
    if(y2 == y1){
        draw_filled_circle(x1, y1, FINGER_WIDTH/2, c);
        int y_line_bottom = y1 - FINGER_WIDTH/2;
        int y_line_top = y1 + FINGER_WIDTH/2;
        gl_draw_line(x1, y_line_bottom, x2, y_line_bottom, c);
        gl_draw_line(x1, y_line_top, x2, y_line_top, c);
        return;
    }
    draw_filled_circle(x1, y1, FINGER_WIDTH/2, c);
    int x_displacement = abs_substraction(x1, x2);
    int y_displacement = abs_substraction(y1, y2);
    
    volatile double angle = atan(y_displacement/x_displacement) * CONVERSION_DEGREE_TO_RADIAN;
    if(x_displacement == 0) angle = 0; //avoid divide by zero
    int x_left_line_bottom = x1 - FINGER_WIDTH/2*(cos(angle));
    int y_line_bottom = y1 - (FINGER_WIDTH/2)*(sin(angle));
    int x_left_line_top = x2 - (FINGER_WIDTH/2) *(cos(angle));
    int y_line_top = y2 - (FINGER_WIDTH/2)*(sin(angle));
    
    gl_draw_line(x_left_line_bottom, y_line_bottom, x_left_line_top, y_line_top, c);
    int x_right_line_bottom = x1 + FINGER_WIDTH/2*(cos(angle));
    int x_right_line_top = x2 + FINGER_WIDTH/2*(cos(angle));
    gl_draw_line(x_right_line_bottom, y_line_bottom, x_right_line_top, y_line_top, c);
}

static void draw_finger(struct FingerDisplay* finger, unsigned int view, int is_thumb) {
    unsigned int *x_positions;
    unsigned int *y_positions;
    if (view == FRONT) {
        x_positions = finger->joints_front_x;
        y_positions = finger->joints_front_y;
    } else {
        x_positions = finger->joints_side_x;
        y_positions = finger->joints_side_y;
    }
    int j = 3;
    if(is_thumb == IS_THUMB)j = 2; //the thumb has less joints so this makes the function only call draw_segment twice.

    for(int i =0; i < j; i++)
        draw_segment(x_positions[i], y_positions[i], x_positions[i+1], y_positions[i+1], finger->color);
    draw_filled_circle(x_positions[j], y_positions[j], FINGER_WIDTH/2, finger->color);
    
}

