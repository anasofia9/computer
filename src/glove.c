#include "glove.h"
#include "mcp3008.h"
#include "printf.h"
#include "timer.h"

static unsigned int STRAIGHT[5];
static unsigned int BENT[5];
static unsigned int RANGE[5];

static unsigned int finger_angle[5];

static void countdown(int seconds) {
    for (int i = seconds; i >= 0; --i) {
        timer_delay(1);
        printf("%d\n", i);
    }
}

void init_glove(void) {
    timer_init();
    mcp3008_init(); 
    printf("Begining the glove initalization routine.\n");
    printf("Please straighten all finters.\n");
    countdown(3);
    for (int finger = THUMB; finger <= PINKY; ++finger) {
        STRAIGHT[finger] = mcp3008_read(finger);
        printf("Finger %d is straignt at %d.\n", finger, STRAIGHT[finger]);
    }
    printf("Please make a fist to maximally bend all sensors.\n");
    countdown(3);
    for (int finger = THUMB; finger <= PINKY; ++finger) {
        BENT[finger] = mcp3008_read(finger);
        printf("Finger %d is straignt at %d.\n", finger, BENT[finger]);
        RANGE[finger] = STRAIGHT[finger] - BENT[finger];
    }
    printf("The glove initalization routine is finished\n");
}

void update_glove(void) {
    for (int finger = THUMB; finger <= PINKY; ++finger) {
        int data = mcp3008_read(finger);
      //  printf("Data for finger %d: %d", finger, data);
        data = (data < BENT[finger]) ? BENT[finger] : ((data > STRAIGHT[finger]) ? STRAIGHT[finger] : data); 
        int angle = ((STRAIGHT[finger] - data) * 90) / RANGE[finger];
        finger_angle[finger] = angle;
    }
}

unsigned int angle(finger j) {
    return finger_angle[(int)j];
}
