
#ifndef ACCELEROMETER_ADXL345_H_
#define ACCELEROMETER_ADXL345_H_

#include "I2C_Master.h"

#define ADXL345_DATAX0 0x32
#define ADXL345_POWER_CTL 0x2D
#define TO_READ (6)      // number of bytes we are going to read each time (two bytes for each axis)
#define ADXL345_ADDR_ALT_LOW  0x53 // ADXL345 address when ALT is connected to LOW

#define ADXL345_Address (0x53)    // ADXL345 device address

void init_Accelerometer() {
	writeTo(ADXL345_POWER_CTL, 8);
}

void writeTo(uint8_t address, uint8_t data) {
	i2c_start(ADXL345_Address);
	i2c_write(address);
	i2c_write(data);
	i2c_stop();
}

void readFrom(int address, uint8_t buffer[], uint8_t length) {
	i2c_start(ADXL345_Address);
	i2c_write(address);
	i2c_stop();

	i2c_start(ADXL345_Address);
	
	uint8_t i;
	for(i = 0; i<length; i++){
		buffer[i] = i2c_readAck();
	}
	i2c_stop();
}

void readAccel(int * Acce) {
	uint8_t _buff[TO_READ]; //6 bytes buffer for saving data read from the device
	readFrom(ADXL345_DATAX0, _buff, TO_READ); //read the acceleration data from the ADXL345
	*(Acce) = (((int)_buff[1]) << 8) | _buff[0]; //x
	*(Acce+1) = (((int)_buff[3]) << 8) | _buff[2]; //y
	*(Acce+2) = (((int)_buff[5]) << 8) | _buff[4]; //z
}

#endif /* ACCELEROMETER_ADXL345_H_ */