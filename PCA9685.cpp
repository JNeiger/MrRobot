// TODO: Implement these functions
#include "PCA9685.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <pigpio.h>
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#include <math.h>
#include <algorithm>

#define PCA9685_SUBADR1 0x2
#define PCA9685_SUBADR2 0x3
#define PCA9685_SUBADR3 0x4

#define PCA9685_MODE1 0x0
#define PCA9685_PRESCALE 0xFE

#define PWM0_ON_L 0x6
#define PWM0_ON_H 0x7
#define PWM0_OFF_L 0x8
#define PWM0_OFF_H 0x9

#define ALLPWM_ON_L 0xFA
#define ALLPWM_ON_H 0xFB
#define ALLPWM_OFF_L 0xFC
#define ALLPWM_OFF_H 0xFD

static void err_handler(int sig) {
	gpioTerminate();
	signal(SIGINT, SIG_DFL);
	kill(getppid(), SIGINT);
	kill(getpid(), SIGINT);
	exit(0);
}

static void exit_handler(void) {
	gpioTerminate();
}

PCA9685::PCA9685(uint8_t addr) {	
	if (gpioInitialise() < 0) {
		printf("Could not initialize the gpio\n\r");
		exit(0);
	}
	
	signal(SIGQUIT, err_handler);
	signal(SIGINT, err_handler);
	signal(SIGTERM, err_handler);
	signal(SIGABRT, err_handler);
	atexit(exit_handler);
	
	// Setup i2c
	i2cHandle = i2cOpen(1, addr, 0);
	
	if (i2cHandle < 0) {
		printf("Error opening the i2c device with error %d\n\r", i2cHandle);
	}
	
	reset();
	// Set to 50 hz for normal motor operation
	setPWMFreq(55);
}

PCA9685::~PCA9685() {
	i2cClose(i2cHandle);
	gpioTerminate();
}

// Resets the PCA9685 chip
void PCA9685::reset() {
	//write8(PCA9685_MODE1, 0x80);
	//if (read8(PCA9685_MODE1) & 0x80) {
	//	write8(PCA9685_MODE1, 0x81);
	//	time_sleep(0.01);
	//}
	write8(PCA9685_MODE1, 0x81);
	time_sleep(.010);
}

// Sets the PWM frequency of the chip
void PCA9685::setPWMFreq(float freq) {
	freq *= 0.9; // Random overshoot due to some conversion factors
	float prescaleval = 25000000;
	prescaleval /= 4096;
	prescaleval /= freq;
	prescaleval -= 1;
	
	uint8_t prescale = floor(prescaleval + 0.5); // Cheap round
	
	uint8_t oldmode = read8(PCA9685_MODE1);
	uint8_t newmode = (oldmode & 0x7F) | 0x10; // Sleep
	write8(PCA9685_MODE1, newmode);
	time_sleep(.1);
	write8(PCA9685_PRESCALE, prescale); // Set the clock scale
	write8(PCA9685_MODE1, oldmode);
	time_sleep(.1);
	write8(PCA9685_MODE1, 0xA1); // Auto increment
	time_sleep(.1);
}

// Sets a PWM channel to have a certain duty cycle
void PCA9685::setPWM(uint8_t channel, float duty) {
	uint16_t val = std::min((uint16_t)floor(duty * 4095 + 0.5), (uint8_t)4095);
	
	printf("Val %d\n\rDuty %f\n\r", val, duty);
	
	if (val == 4095) {
		setPWM(channel, 4096, 0);
	} else if (val == 0) {
		setPWM(channel, 0, 4096);
	} else {
		setPWM(channel, 0, val);
	}
}

// Sets a PWM channel to switch on and off on specific counts
void PCA9685::setPWM(uint8_t channel, uint16_t on, uint16_t off) {
	unsigned char base = PWM0_ON_L + 4*channel;
	i2cWriteByteData(i2cHandle, base, (unsigned char)on);
	i2cWriteByteData(i2cHandle, base+1, (unsigned char)(on >> 8));
	i2cWriteByteData(i2cHandle, base+2, (unsigned char)off);
	i2cWriteByteData(i2cHandle, base+3, (unsigned char)(off >> 8));
}

// Reads 8 bits from a register
uint8_t PCA9685::read8(uint8_t reg) {
	return i2cReadByteData(i2cHandle, reg);
}

// Writes 8 bits to a register
void PCA9685::write8(uint8_t reg, uint8_t data) {
	int ret;
	if ((ret = i2cWriteByteData(i2cHandle, reg, data)) < 0) {
		printf("Coudn't write to I2C device: Error %d\n\r", ret);
	}
}
