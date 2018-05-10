#ifndef GLOVE_H
#define GLOVE_H

typedef unsigned int finger;

#define THUMB     0
#define POINTER   1
#define MIDDLE    2
#define RING      3
#define PINKY     4

void init_glove(void);
void update_glove(void);
unsigned int angle(finger f);

#endif
