#pragma once
#include <errno.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/hci.h>
#include <bluetooth/hci_lib.h>
#include <bluetooth/rfcomm.h>

// lB defines
#define RS      0x80
#define LS      0x40
#define SELECT  0x20
#define START   0x10
#define D_RIGHT 0x08
#define D_LEFT  0x04
#define D_DOWN  0x02
#define D_UP    0x01

// uB defines
#define Y  0x80
#define X  0x40
#define B  0x20
#define A  0x10
#define RB 0x02
#define LB 0x01

//Mirror of the packet in windows
struct bt_packet{
	// Left/Right Trigger
	unsigned char lT = 0;
	unsigned char rT = 0;
	
	//Left/Right X/Y axis
	char lX = 0;
	char lY = 0;
	char rX = 0;
	char rY = 0;
	
	// Lower/Upper Buttons
	unsigned char lB = 0;
	unsigned char uB = 0;
};

class BluetoothServer {
public:
	BluetoothServer();
	~BluetoothServer();
	
	void scanForDevices();
	void listenForClient();
	int readData(char* buffer, int len);
	
private:
	int me;
	int them;
};
