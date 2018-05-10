#include "servo.h"
#include "glove.h"
#include "printf.h"

const unsigned int DATA_PEACE[5] = {15, 4, 4, 15, 15};
const unsigned int DATA_SHAKA[5] = {4, 15, 15, 15, 4};
const unsigned int DATA_BIRD[5] = {6, 15, 4, 15, 15};
const unsigned int DATA_ROCK[5] = {4, 4, 15, 15, 4};
const unsigned int DATA_OPEN[5] = {4, 4, 4, 4, 4};

void init_hand(){
  servo_init();

}

static unsigned int angle_to_pwm(unsigned int angle){
  return angle*(12/90) + 4;
}

void update_finger(unsigned int fingerNum){
   unsigned int newPwm = angle(fingerNum);
   servo_set_pwm(fingerNum, newPwm);
}


void update_hand(){
  for(int i = 0; i < 5; i++){
    update_finger(i);
  }
}

void go_to_pose(unsigned int pose_num){
  unsigned int *pose;
  switch(pose_num){
    case 0:
      pose = DATA_PEACE;
      printf("pose = peace\n");
      break;
    case 1: 
      pose = DATA_SHAKA;
      printf("pose = shaka\n");
      break;
    case 2:
      pose = DATA_BIRD;
      printf("pose = bird\n");
      break;
    case 3:
      pose = DATA_ROCK;
      printf("pose = rock\n");
      break;
    case 4:
      pose = DATA_OPEN;
      printf("pose = open\n");
      break;
  }

  servo_set_pwm(THUMB, pose[THUMB]);
  servo_set_pwm(POINTER, pose[POINTER]);
  servo_set_pwm(MIDDLE, pose[MIDDLE]);
  servo_set_pwm(RING, pose[RING]);
  servo_set_pwm(PINKY, pose[PINKY]);
}
