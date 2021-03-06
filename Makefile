all: robot.o PCA9685.o Motor.o BluetoothServer.o
	g++ robot.o PCA9685.o Motor.o BluetoothServer.o -o robot -lbluetooth -lpigpio -lrt
	
robot.o: robot.cpp
	g++ -c robot.cpp -lbluetooth -lpigpio -lrt
	
PCA9685.o: PCA9685.cpp
	g++ -c PCA9685.cpp -lbluetooth -lpigpio -lrt
	
Motor.o: Motor.cpp
	g++ -c Motor.cpp -lbluetooth -lpigpio -lrt
	
BluetoothServer.o: BluetoothServer.cpp
	g++ -c BluetoothServer.cpp -lbluetooth -lpigpio -lrt
