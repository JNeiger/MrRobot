// Single motor driver for the TB6612FNG H-Bridge
// TODO: Change the IN1/2 to actual pins
#pragma once
#include "PCA9685.hpp"

class Motor {
public:
	// Assumes the extender has already been set up
	Motor(PCA9685* pwmExtender, uint8_t channel, unsigned char IN1, unsigned char IN2);

	// From -1 to 1 which is full backwards to full forwards
	void set(float percent);

private:
	PCA9685* extender;
	unsigned char IN1;
	unsigned char IN2;
	uint8_t channel;
};
