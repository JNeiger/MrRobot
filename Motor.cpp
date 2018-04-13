#include "Motor.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>

Motor::Motor(PCA9685* pwmExtender, uint8_t channel, unsigned char IN1, unsigned char IN2) {
	// Assume pigpio has already been initialized
	int ret[2];
	
	ret[0] = gpioSetMode(IN1, PI_OUTPUT);
	ret[1] = gpioSetMode(IN2, PI_OUTPUT);
	
	if (ret[0] != 0 || ret[1] != 0) {
		printf("Motor IN1 or IN2 failed with codes %d, %d\n\r", ret[0], ret[1]);
	}
	
	gpioWrite(IN1, 1);
	gpioWrite(IN2, 1);
	
	extender = pwmExtender;
	this->IN1 = IN1;
	this->IN2 = IN2;
	this->channel = channel;
}

// From -1 to 1 which is full backwards to full forwards
void Motor::set(float percent) {
	unsigned char i1, i2;
	float duty;
	
	if (std::abs(percent) < 0.001) {
		i1 = 1;
		i2 = 1;
		duty = 0.5;
	} else if (percent > 0) {
		i1 = 1;
		i2 = 0;
		duty = percent;
	} else { // percent < 0
		i1 = 0;
		i2 = 1;
		duty = -percent;
	}
	
	gpioWrite(IN1, i1);
	gpioWrite(IN2, i2);
	extender->setPWM(channel, duty);
}
