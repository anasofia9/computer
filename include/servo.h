#ifndef SERVO_H
#define SERVO_H


void servo_init(void);
void servo_set_pwm(unsigned int servoNum, unsigned int percentOn);
void servo_set_position(unsigned int servoNum, unsigned int pwm);

#endif
