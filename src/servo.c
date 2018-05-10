#include "servo.h"
#include "i2c.h"
#include "gpio.h"
#include "uart.h"
#include "printf.h"
#include "timer.h"

#define BASE_ADDR  0x40

enum pca9685_registers{
  MODE1, MODE2, SUBADR1, SUBADR2, SUBADR3, ALLCALLADR,
  LED0_ON_L, LED0_ON_H, LED0_OFF_L, LED0_OFF_H,
  LED1_ON_L, LED1_ON_H, LED1_OFF_L, LED1_OFF_H,
  LED2_ON_L, LED2_ON_H, LED2_OFF_L, LED2_OFF_H,
  LED3_ON_L, LED3_ON_H, LED3_OFF_L, LED3_OFF_H,
  LED4_ON_L, LED4_ON_H, LED4_OFF_L, LED4_OFF_H,
  LED5_ON_L, LED5_ON_H, LED5_OFF_L, LED5_OFF_H,
  LED6_ON_L, LED6_ON_H, LED6_OFF_L, LED6_OFF_H,
  LED7_ON_L, LED7_ON_H, LED7_OFF_L, LED7_OFF_H,
  LED8_ON_L, LED8_ON_H, LED8_OFF_L, LED8_OFF_H,
  LED9_ON_L, LED9_ON_H, LED9_OFF_L, LED9_OFF_H,
  LED10_ON_L, LED10_ON_H, LED10_OFF_L, LED10_OFF_H,
  LED11_ON_L, LED11_ON_H, LED11_OFF_L, LED11_OFF_H,
  LED12_ON_L, LED12_ON_H, LED12_OFF_L, LED12_OFF_H,
  LED13_ON_L, LED13_ON_H, LED13_OFF_L, LED13_OFF_H,
  LED14_ON_L, LED14_ON_H, LED14_OFF_L, LED14_OFF_H,
  LED15_ON_L, LED15_ON_H, LED15_OFF_L, LED15_OFF_H
};

#define ALL_LED_ON_L 250
#define ALL_LED_ON_H 251
#define ALL_LED_OFF_L 252
#define ALL_LED_OFF_H 253
#define PRE_SCALE 254


static void servo_set_one_reg(unsigned int regNum, char data);
static void servo_set_freq(unsigned int freq);
static void servo_set_four_regs(unsigned int servoNum, char onL, char onH, char offL, char offH);

void servo_init(void){
  i2c_init();
  timer_init();
  servo_set_one_reg(MODE1, 0x80);
  timer_delay_ms(10);
  servo_set_freq(60);
}

static void servo_set_freq(unsigned int freq){
  freq *= .95;
  float prescaleval = 25000000;
  prescaleval = prescaleval/4096;
  prescaleval = prescaleval/freq;
  prescaleval -= 1;
  unsigned int finalPrescale = prescaleval + .5;
  char oldMode = MODE1;
  //printf("%d\n", oldMode);
  i2c_read(BASE_ADDR, &oldMode, 1);
  char newMode = (oldMode&0x7f)|0x10;
  servo_set_one_reg(MODE1, newMode);
  servo_set_one_reg(PRE_SCALE, finalPrescale);
  servo_set_one_reg(MODE1, oldMode);
  oldMode = oldMode | 0xa0;
  servo_set_one_reg(MODE1, oldMode);

}

static void servo_set_one_reg(unsigned int regNum, char data){
  char array[2];
  array[0] = regNum;
  array[1] = data;
  i2c_write(BASE_ADDR, array, 2);
}

static void servo_set_four_regs(unsigned int servoNum, char onL, char onH, char offL, char offH){
  char servoReg = LED0_ON_L + 4*servoNum;
  char data[5];
  data[0] = servoReg;
  data[1] = onL;
  data[2] = onH;
  data[3] = offL;
  data[4] = offH;
  i2c_write(BASE_ADDR, data, 5);
}

void servo_set_pwm(unsigned int servoNum, unsigned int percentOn){
  if(percentOn > 15 || percentOn < 3){
    return;
  }
  unsigned int timeTurnOff = 40.96*percentOn;
  char offH = timeTurnOff >> 8;
  char offL = timeTurnOff & 0xff;
  servo_set_four_regs(servoNum, 0, 0, offL, offH); 

}

void servo_set_position(unsigned int servoNum, unsigned int pwm){
  for(int i = 0; i < pwm; i++){
    servo_set_pwm(servoNum, i);
  }
}
