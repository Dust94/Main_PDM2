
#ifndef MAGNETOMETER_HMC5883L_H_
#define MAGNETOMETER_HMC5883L_H_

#include "Basic.h" // min, max
#include "I2C_Master.h"
//#include "uart_michel.h"
//#include "Serial.h"
#include <util/delay.h>

#define HMC5883L_Address 0x1E // 7-bit address
#define HMC5883L_WRITE 0x3C // Write address. 7-bit address (0x1E) + 1 bit write identifier
#define HMC5883L_READ 0x3D // Read address. 7-bit address (0x1E) + 1 bit read identifier

#define ConfigurationRegisterA 0x00
#define ConfigurationRegisterB 0x01
#define ModeRegister 0x02
#define DataOutputX_MSB 0x03

#define Continuous_Mode 0x00
#define Single_Mode 0x01
#define Idle_Mode 0x02 // Or 0x03

#define compass_XY_excitation 1160 // The magnetic field excitation in X and Y direction during Self Test (Calibration)
#define compass_Z_excitation 1080  // The magnetic field excitation in Z direction during Self Test (Calibration)

#define compass_rad2degree 57.3
#define compass_cal_x_offset 116   // Manually calculated offset in X direction
#define compass_cal_y_offset 225   // Manually calculated offset in Y direction
#define compass_cal_x_gain 1.1     // Stored Gain offset at room temperature
#define compass_cal_y_gain 1.12    // Stored Gain offset at room temperature

float Offset[3] = {0,0,0};
float GainError[3] = {1,1,1};
float m_Scale = 1;


void Write_Compass(uint8_t address, uint8_t data){
	i2c_start(HMC5883L_WRITE);
	i2c_write(address);
	i2c_write(data);
	i2c_stop();
}

void Compass_SetScale(float gauss){
	uint8_t regValue = 0x00;
	if(gauss == 0.88){
		regValue = 0x00;
		m_Scale = 0.73;
	}else if(gauss == 1.3){
		regValue = 0x01;
		m_Scale = 0.92;
	}else if(gauss == 1.9){
		regValue = 0x02;
		m_Scale = 1.22;
	}else if(gauss == 2.5){
		regValue = 0x03;
		m_Scale = 1.52;
	}else if(gauss == 4.0){
		regValue = 0x04;
		m_Scale = 2.27;
	}else if(gauss == 4.7){
		regValue = 0x05;
		m_Scale = 2.56;
	}else if(gauss == 5.6){
		regValue = 0x06;
		m_Scale = 3.03;
	}else if(gauss == 8.1){
		regValue = 0x07;
		m_Scale = 4.35;
	}
	regValue = regValue << 5; // Setting is in the top 3 bits of the register.
	Write_Compass(ConfigurationRegisterB, regValue);
}

void init_HMC5883L(void){
	Write_Compass(ConfigurationRegisterA,0x70); //(number of samples averaged = 8) and (Data Output Rate Bits = 15 default)
	Compass_SetScale(1.3); //Gain Configuration Bits. Sensor Field Range=±1.3(Ga), Gain=390(LSb/Gauss), Resolution=2.56(mG/LSb)
	Write_Compass(ModeRegister,Continuous_Mode);
}



void Read_Compass(uint8_t buffer[], uint8_t length){
	
	
	i2c_start_wait(HMC5883L_WRITE);
	i2c_write(DataOutputX_MSB); //set pointer to X-axis MSB	
	i2c_stop();
	
	i2c_start(HMC5883L_READ);
	uint8_t i;
	for(i = 0; i<length-1; i++){
		buffer[i] = i2c_readAck();
	}
	buffer[i] = i2c_readNak();
	i2c_stop();
}

void Compass_ReadRawAxis(int * Mag_Raw){
	uint8_t buffer[6];
	Read_Compass(buffer, 6);
	*(Mag_Raw) = (buffer[0] << 8) | buffer[1]; //x
	*(Mag_Raw+2) = (buffer[2] << 8) | buffer[3]; //z
	*(Mag_Raw+1) = (buffer[4] << 8) | buffer[5]; //y	
}

void Compass_ReadScaledAxis(float * Mag_Scaled){
	int Mag_Raw[3];
	Compass_ReadRawAxis(&Mag_Raw[0]);
	*(Mag_Scaled) = Mag_Raw[0] * m_Scale; //x
	*(Mag_Scaled+1) = Mag_Raw[1] * m_Scale; //y
	*(Mag_Scaled+2) = Mag_Raw[2] * m_Scale; //z
}


// This Function calculates the offset in the Magnetometer using Positive and Negative bias Self test capability
// This function updates X_offset Y_offset and Z_offset Global variables. Call Initialize before 

void compass_offset_calibration(int select){
	int Mag_Raw[3];
	float Mag_Scaled[3];
	
	// Gain offset estimation
	if ((select == 1) | (select == 3)){ // User input in the function
		Write_Compass(ConfigurationRegisterA, 0b01110001); // Configuring the Control register for Positive Bias mode
		/* bit configuration = 0 A A DO2 DO1 DO0 MS1 MS2
		A A                        DO2 DO1 DO0      Sample Rate [Hz]      MS1 MS0    Measurment Mode
		0 0 = No Average            0   0   0   =   0.75                   0   0   = Normal  
		0 1 = 2 Sample average      0   0   1   =   1.5                    0   1   = Positive Bias
		1 0 = 4 Sample Average      0   1   0   =   3                      1   0   = Negative Bais
		1 1 = 8 Sample Average      0   1   1   =   7.5                    1   1   = -
		1   0   0   =   15 (Default)
		1   0   1   =   30
		1   1   0   =   75
		1   1   1   =   -
		*/
		Compass_ReadRawAxis(&Mag_Raw[0]); // Disregarding the first data
		while((Mag_Raw[0]<200) | (Mag_Raw[1]<200) | (Mag_Raw[2]<200)){   // Making sure the data is with Positive baised
		Compass_ReadRawAxis(&Mag_Raw[0]); // Reading the Positive baised Data
		} //Fin While
		Mag_Scaled[0] = Mag_Raw[0] * m_Scale;
		Mag_Scaled[1] = Mag_Raw[1] * m_Scale;
		Mag_Scaled[2] = Mag_Raw[2] * m_Scale;
		// Offset = 1160 - Data_positive
		GainError[0] = (float)compass_XY_excitation/Mag_Scaled[0];
		GainError[1] = (float)compass_XY_excitation/Mag_Scaled[1];
		GainError[2] = (float)compass_Z_excitation/Mag_Scaled[2];
		
		Write_Compass(ConfigurationRegisterA, 0b01110010); // Configuring the Control register for Negative Bias mode
		Compass_ReadRawAxis(&Mag_Raw[0]); // Disregarding the first data
		while((Mag_Raw[0]>-200) | (Mag_Raw[1]>-200) | (Mag_Raw[2]>-200)){  // Making sure the data is with negative baised
		Compass_ReadRawAxis(&Mag_Raw[0]); // Reading the Negative baised Data
		} //Fin While
		Mag_Scaled[0] = Mag_Raw[0] * m_Scale;
		Mag_Scaled[1] = Mag_Raw[1] * m_Scale;
		Mag_Scaled[2] = Mag_Raw[2] * m_Scale;
		// Taking the average of the offsets
		GainError[0] = (float)( (compass_XY_excitation/abs(Mag_Scaled[0])) + GainError[1] )/2;
		GainError[1] = (float)( (compass_XY_excitation/abs(Mag_Scaled[1])) + GainError[2] )/2;
		GainError[2] = (float)( (compass_Z_excitation/abs(Mag_Scaled[2])) + GainError[3] )/2;
	} //Fin If
	
	Write_Compass(ConfigurationRegisterA, 0b01111000); // Configuring the Control register for normal mode
	// Offset estimation
	if ((select == 2) | (select == 3)){
		//uart_println(0, "Calibrating the Magnetometer ....... Offset");
		//Serial_write("\n\r");
		//uart_println(0, "Please rotate the magnetometer 2 or 3 times in complete circules with in one minute .............");
		//Serial_write("\n\r");
		uint8_t i;
		for(i=0; i<10; i++){   
			Compass_ReadRawAxis(&Mag_Raw[0]); // Disregarding first few data
		}
		
		float x_max=-4000,y_max=-4000,z_max=-4000; 
		float x_min=4000,y_min=4000,z_min=4000;
		
		//uart_println(0,"Starting Debug data in "); //Serial_write("\n\r");
		//_delay_ms(1000); uart_println(0, "3"); //Serial_write("\n\r");
		//_delay_ms(1000); uart_println(0,"2"); //Serial_write("\n\r");
		//_delay_ms(1000); uart_println(0,"1"); //Serial_write("\n\r");
		int t = 0;
		int j;
		for(j=0; j<=25000; j++){ //Moreless half minute
			Compass_ReadRawAxis(&Mag_Raw[0]);
			Mag_Scaled[0] = (float)Mag_Raw[0]*m_Scale*GainError[0];
			Mag_Scaled[1] = (float)Mag_Raw[1]*m_Scale*GainError[1];
			Mag_Scaled[2] = (float)Mag_Raw[2]*m_Scale*GainError[2];
			x_max = max(x_max,Mag_Scaled[0]);
			y_max = max(y_max,Mag_Scaled[1]);
			z_max = max(z_max,Mag_Scaled[2]);
			x_min = min(x_min,Mag_Scaled[0]);
			y_min = min(y_min,Mag_Scaled[1]);
			z_min = min(z_min,Mag_Scaled[2]);
			//uart_print(0,"j> ");
			//uart_println(0,itoa(j,buffer, 10));			
			if(j == t){
				char ts[10];
				//uart_print(0,"ts: ");
				//uart_println(0,itoa(t, ts,10));				
				//TxBCD((int)t/1000000); Serial_write("\n\r");
				t+= 100;
				_delay_ms(10);
			} // Fin If			
		} // Fin For
		Offset[0] = ((x_max-x_min)/2)-x_max;
		Offset[1] = ((y_max-y_min)/2)-y_max;
		Offset[2] = ((z_max-z_min)/2)-z_max;
	//uart_println(0,"End Debug -- (Offset Calibration)");	//Serial_write("\n\r");
	} // Fin If
}

void Compass_Scalled_Reading(float * Mag_Scaled){
  int Mag_Raw[3];
  Compass_ReadRawAxis(&Mag_Raw[0]);
  *(Mag_Scaled) = Mag_Raw[0] * m_Scale * GainError[0] + Offset[0]; //x
  *(Mag_Scaled+1) = Mag_Raw[1] * m_Scale * GainError[1] + Offset[1]; //y
  *(Mag_Scaled+2) = Mag_Raw[2] * m_Scale * GainError[2] + Offset[2]; //z
}

float compass_heading(){
	float Mag_Scaled[3];
	Compass_Scalled_Reading(&Mag_Scaled[0]);
	int heading = 0;
	
	if (Mag_Scaled[1]>0) {
		heading = (int)90-atan(Mag_Scaled[0]/Mag_Scaled[1])*180/PI;
	}
	if (Mag_Scaled[1]<0) {
		heading = (int)270-atan(Mag_Scaled[0]/Mag_Scaled[1])*180/PI;
	}
	if ((Mag_Scaled[1]==0) & (Mag_Scaled[0]<0))	heading = 180;
	return heading;
}







#endif /* MAGNETOMETER_HMC5883L_H_ */