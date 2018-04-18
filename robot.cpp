#include <cmath>
#include <algorithm>
#include <pigpio.h>
#include "BluetoothServer.hpp"
#include "PCA9685.hpp"
#include "Motor.hpp"

// 4/17 27/22 10/9 11/5
#define LEFT_DRIVE_PWM_CHANNEL 3
#define LEFT_DRIVE_IN1 10
#define LEFT_DRIVE_IN2 9

#define RIGHT_DRIVE_PWM_CHANNEL 0
#define RIGHT_DRIVE_IN1 4
#define RIGHT_DRIVE_IN2 17

#define INTAKE_PWM_CHANNEL 1
#define INTAKE_IN1 27
#define INTAKE_IN2 22

#define KICKER_PWM_CHANNEL 2
#define KICKER_IN1 11
#define KICKER_IN2 5

// Out of 100%
#define DEADZONE 0.1
// Out of 100%
#define MAX_SPEED 1.00f

// Max number of while loops cycles the solinoid can stay on in a row
// Each while loop executes every ~50 ms
// 10 while loops is about ~500 ms
#define MAX_ON_CNT 10
// Min number of while loops cycles the solinoid must stay off to cool down
// Each while loop executes every ~50 ms
// 30 while loops is about ~1.5 secs
#define MIN_OFF_CNT 30

int main(int argc, char **argv)
{
	BluetoothServer bt;
	struct bt_packet bluetoothPacket;
	PCA9685 extender;
	Motor *driveMotors = (Motor*)malloc(2*sizeof(Motor));	
	driveMotors[0] = Motor(&extender, LEFT_DRIVE_PWM_CHANNEL,  LEFT_DRIVE_IN1,  LEFT_DRIVE_IN2);
	driveMotors[1] = Motor(&extender, RIGHT_DRIVE_PWM_CHANNEL, RIGHT_DRIVE_IN1, RIGHT_DRIVE_IN2);
	//Motor intake(&extender, INTAKE_PWM_CHANNEL, INTAKE_IN1, INTAKE_IN2);
	//Motor kicker(&extender, KICKER_PWM_CHANNEL, KICKER_IN1, KICKER_IN2);
	
	// Controls the timeout for the solinoid
	// Turns on for MAX_ON_CNT cycles, then off for at least MIN_OFF_CNT cycles
	int onCtr = 0;
	int offCtr = 0;
	bool kickOn = false;
	
	// Set the motors to 0 so they don't randomly start moving when you turn it on
	driveMotors[0].set(0);
	driveMotors[1].set(0);
	
	// Blocking call so this will wait until someone connects
	bt.listenForClient();
	
	// Once we get a client connected, read the input data
	// and do whatever we need to do based on that
	while (true) {
		// Read the controller data from bluetooth
		int bytes_read = bt.readData((char*)&bluetoothPacket, sizeof(struct bt_packet));
		//printf("Read %d bytes\n\r", bytes_read);
		
		// Convert the axis input to the range -1 to 1 with a small deadzone
		// The max is needed since some of them are from -128 to 127
		// so dividing by 127 may result in a number just slightly bigger than -1
		float lx = std::max((int8_t)bluetoothPacket.lX / 127.0f, -1.0f);
		float ly = std::max((int8_t)bluetoothPacket.lY / 127.0f, -1.0f);
		float rx = std::max((int8_t)bluetoothPacket.rX / 127.0f, -1.0f);
		float ry = std::max((int8_t)bluetoothPacket.rY / 127.0f, -1.0f);
		float lt = (uint8_t)bluetoothPacket.lT / 255.0f;
		float rt = (uint8_t)bluetoothPacket.rT / 255.0f;
		lx *= (std::abs(lx) >= DEADZONE);
		ly *= (std::abs(ly) >= DEADZONE);
		rx *= (std::abs(rx) >= DEADZONE);
		ry *= (std::abs(ry) >= DEADZONE);
		lt *= (std::abs(lt) >= DEADZONE);
		rt *= (std::abs(rt) >= DEADZONE);
		
		// Use all the input from the controller
		//printf("(%-10.2f, %-10.2f) (%-10.2f, %-10.2f) (%-10.2f, %-10.2f)\n\r", lx, ly, rx, ry, lt, rt);
		
		// Convert left and right stick arcade controls to tank drive
		float left_speed  = std::max(-1.0f, std::min(1.0f, ly + rx)) * MAX_SPEED;
		float right_speed = std::max(-1.0f, std::min(1.0f, ly - rx)) * MAX_SPEED;
		
		// When going backwards, it feels better when the controls are flipped
		
		driveMotors[0].set(left_speed);
		driveMotors[1].set(right_speed);
		//printf("%-10.2f %-10.2f\n\r", left_speed, right_speed);
		
		//Control the intake using the two triggers
		// Left trigger goes backward, right trigger goes forward
		//intake.set(rt - lt);
		//printf("%-10.2f\n\r", rt - lt);
		
		// Y Button being pushed
		if ((bluetoothPacket.uB & Y) != 0) {
			printf("Y Button being pushed\n\r");
		}
		
		// X Button being pushed
		if ((bluetoothPacket.uB & X) != 0) {
			printf("X Button being pushed\n\r");
		}
		
		// B Button being pushed
		if ((bluetoothPacket.uB & B) != 0) {
			printf("B Button being pushed\n\r");
		}
		
		// A Button being pushed
		if ((bluetoothPacket.uB & A) != 0) {
			//printf("A Button being pushed\n\r");
			kickerOn = true;
		}
		
		if (kickerOn && onCtr < MAX_ON_CNT) {
			//kicker.set(1.0f);
			onCtr++;
			printf("Kicker On %d\n\r",  onCtr);
		} else if (kickerOn && onCtr >= MAX_ON_CNT && offCtr < MIN_OFF_CNT) {
			//kicker.set(0.0f);
			printf("Kicker Off %d\n\r",  offCtr);
			offCtr++;
		} else if (kickerOn) {
			printf("Ready for another Kick\n\r");
			kickerOn = false;
			onCtr = 0;
			offCtr = 0;
		}
		
		// RB Button being pushed
		if ((bluetoothPacket.uB & RB) != 0) {
			printf("RB Button being pushed\n\r");
		}
		
		// LB Button being pushed
		if ((bluetoothPacket.uB & LB) != 0) {
			printf("LB Button being pushed\n\r");
		}
		
		// Right Stick Button being pushed
		if ((bluetoothPacket.lB & RS) != 0) {
			printf("RS Button being pushed\n\r");
		}
		
		// Left Stick Button being pushed
		if ((bluetoothPacket.lB & LS) != 0) {
			printf("lS Button being pushed\n\r");
		}
		
		// Select Button being pushed
		if ((bluetoothPacket.lB & SELECT) != 0) {
			printf("SELECT Button being pushed\n\r");
		}
		
		// Start Button being pushed
		if ((bluetoothPacket.lB & START) != 0) {
			printf("START Button being pushed\n\r");
		}
		
		// D-Pad Right Button being pushed
		if ((bluetoothPacket.lB & D_RIGHT) != 0) {
			printf("DR Button being pushed\n\r");
		}
		
		// D-Pad Left Button being pushed
		if ((bluetoothPacket.lB & D_LEFT) != 0) {
			printf("DL Button being pushed\n\r");
		}
		
		// D-Pad Down Button being pushed
		if ((bluetoothPacket.lB & D_DOWN) != 0) {
			printf("DD Button being pushed\n\r");
		}
		
		// D-Pad Up Button being pushed
		if ((bluetoothPacket.lB & D_UP) != 0) {
			printf("DU Button being pushed\n\r");
		}
	}
	
    return 0;
}

