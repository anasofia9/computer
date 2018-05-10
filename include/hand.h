#ifndef HAND_H
#define HAND_H

#define PEACE 0
#define SHAKA 1
#define BIRD 2
#define ROCK 3
#define OPEN 4


void init_hand();
void update_finger(unsigned int fingerNum);
void update_hand();
void go_to_pose(int pos);

#endif
