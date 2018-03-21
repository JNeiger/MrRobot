// 16 Channel PWM extender using I2C
#pragma once
#include <stdint.h>

class PCA9685 {
public:
	PCA9685(uint8_t addr = 0x40);
	~PCA9685();
	
	// Resets the PCA9685 chip
	void reset();
	
	// Sets the PWM frequency of the chip
	void setPWMFreq(float freq);
	
	// Sets a PWM channel to have a certain duty cycle
	void setPWM(uint8_t channel, float duty);
	
	// Sets a PWM channel to switch on and off on specific counts
	void setPWM(uint8_t channel, uint16_t on, uint16_t off);

private:
	// Device address
	int i2cHandle;
	
	// Reads 8 bits from a register
	uint8_t read8(uint8_t reg);
	
	// Writes 8 bits to a register
	void write8(uint8_t reg, uint8_t data);
};
