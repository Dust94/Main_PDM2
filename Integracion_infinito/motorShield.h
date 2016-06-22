#ifndef MOTORSHIELD_H_
#define MOTORSHIELD_H_

#include <avr/io.h>‬

#define MOTOR_A 1
#define MOTOR_B 2
#define MOTOR_C 3
#define MOTOR_D 4

#define Stopped 0
#define Forward 1
#define Backward 2
#define TurnRight 3
#define TurnLeft 4

#define ERROR 'E'
#define sbi(port, bit) (port) |= (1 << (bit))
#define cbi(port, bit) (port) &= ~(1 << (bit))

int leftSpeed = 0;
int rightSpeed = 0;
int state = 0;

int actualSpeed = 0;

void motorShield_init(void)
{
	
	DDRB = (1 << DDB5)	|	(1 << DDB6);	// pin OC1A (PB5) como salida - pin OC1B (PB6) como salida
	DDRE = (1 << DDE3)	|	(1 << DDE5);	// pin OC3A (PE3) como salida - pin OC3C (PE5) como salida
	
	DDRA = 0xFF; // Puerto A como salida utilizado para conmutacion de motores
	
	PORTB = (1 << PB5)	|	(1 << PB6);		// Inicialmente a cero
	PORTE = (1 << PE3)	|	(1 << PE5);		// Inicialmente a cero
	
	PORTA = 0x00;
	
	TCCR1A |= (0 << WGM11)| (0 << WGM10); // set PWM Mode 8
	TCCR1B |= (1 << WGM13) | (0 << WGM12) | (0 << CS12) | (0 << CS11)| (1 << CS10); // set prescaler to 8 and starts PWM
	
	TCCR3A |= (0 << WGM31)| (0 << WGM30); // set PWM Mode 8
	TCCR3B |= (1 << WGM33) | (0 << WGM32) | (0 << CS32) | (0 << CS31)| (1 << CS30); // set prescaler to 8 and starts PWM
	
	sbi(TCCR1A,COM1A1);
	sbi(TCCR1A,COM1B1);
	
	sbi(TCCR3A,COM3A1);
	sbi(TCCR3A,COM3C1);
	
	ICR1 = 800;
	ICR3 = 800;
	
}


void motorShield_setSpeed(uint8_t motor, int speed)
{
	 unsigned char reverse = 0;
	
	if ( speed < 0 )
	{
		reverse = 1;
		speed = (-1)*speed;
	}
	
	if ( speed > 100 )
	{
		speed = 100;
	}
	
	
	if (reverse)
	{
		switch(motor)
		{
			case 1:
						OCR1A= speed * 8; // Shield 2
						cbi(PORTA,PA4); //Motor A salida pin PA5 a M1INA
						sbi(PORTA,PA5);	//Motor A salida pin PA4 a M1INB
						leftSpeed = speed;
						break;
			
			case 2:
						OCR1B= speed * 8; // Shield 2
						cbi(PORTA,PA7); //Motor B salida pin PA6 a M2INA
						sbi(PORTA,PA6);	//Motor B salida pin PA7 a M2INB
						rightSpeed = speed;
						break;
						
			case 3:
			
						OCR3A= speed * 8; // Shield 1
						cbi(PORTA,PA3); //Motor A salida pin PA3 a M1INA
						sbi(PORTA,PA2);	//Motor A salida pin PA2 a M1INB
						leftSpeed = speed;
						break;
			
			case 4:
			
						OCR3C= speed * 8; // Shield 1
						cbi(PORTA,PA0); //Motor B salida pin PA1 a M2INA
						sbi(PORTA,PA1);	//Motor B salida pin PA0 a M2INB
						rightSpeed = speed;
						break;
		}
	} 
	
	else
	{
		switch(motor)
		{
			case 1:
						OCR1A= speed * 8; // Shield 2
						sbi(PORTA,PA4); //Motor A salida pin PA5 a M1INA
						cbi(PORTA,PA5);	//Motor A salida pin PA4 a M1INB
						leftSpeed = speed;
						break;
			
			case 2:
						OCR1B= speed * 8; // Shield 2
						sbi(PORTA,PA7); //Motor B salida pin PA6 a M2INA
						cbi(PORTA,PA6);	//Motor B salida pin PA7 a M2INB
						rightSpeed = speed;
						break;
			
			case 3:
			
						OCR3A= speed * 8; // Shield 1
						sbi(PORTA,PA3); //Motor A salida pin PA3 a M1INA
						cbi(PORTA,PA2);	//Motor A salida pin PA2 a M1INB
						leftSpeed = speed;
						break;
			
			case 4:
			
						OCR3C= speed * 8; // Shield 1
						sbi(PORTA,PA0); //Motor B salida pin PA1 a M2INA
						cbi(PORTA,PA1);	//Motor B salida pin PA0 a M2INB
						rightSpeed = speed;
						break;
		}
	}
}


void motorShield_setBrake(uint8_t motor, int speed)
{
	if ( speed < 0 )
	{
		speed = (-1)*speed;
	}
	
	if ( speed > 100 )
	{
		speed = 100;
	}
	
	switch(motor)
	{
			case 1:
						OCR1A= speed * 8; // Shield 2
						cbi(PORTA,PA4); //Motor A salida pin PA5 a M1INA
						cbi(PORTA,PA5);	//Motor A salida pin PA4 a M1INB
						leftSpeed = speed;
						break;
		
			case 2:
						OCR1B= speed * 8; // Shield 2
						cbi(PORTA,PA7); //Motor B salida pin PA6 a M2INA
						cbi(PORTA,PA6);	//Motor B salida pin PA7 a M2INB
						rightSpeed = speed;
						break;
		
			case 3:
		
						OCR3A= speed * 8; // Shield 1
						cbi(PORTA,PA3); //Motor A salida pin PA3 a M1INA
						cbi(PORTA,PA2);	//Motor A salida pin PA2 a M1INB
						leftSpeed = speed;
						break;
		
			case 4:
		
						OCR3C= speed * 8; // Shield 1
						cbi(PORTA,PA0); //Motor B salida pin PA1 a M2INA
						cbi(PORTA,PA1);	//Motor B salida pin PA0 a M2INB
						rightSpeed = speed;
						break;
	}
	
}

void motorShield_move(int movement, int speed)
{
	switch(movement)
	{
	
	case 1:
					motorShield_setSpeed(MOTOR_A,-speed);
					motorShield_setSpeed(MOTOR_B,speed);
					motorShield_setSpeed(MOTOR_C,speed);
					motorShield_setSpeed(MOTOR_D,speed);
					state = Forward;
					break;
	
	case 2:
					motorShield_setSpeed(MOTOR_A,speed);
					motorShield_setSpeed(MOTOR_B,-speed);
					motorShield_setSpeed(MOTOR_C,-speed);
					motorShield_setSpeed(MOTOR_D,-speed);
					state = Backward;
					break;
	
	case 3:
	
					motorShield_setSpeed(MOTOR_A,-speed);
					motorShield_setSpeed(MOTOR_B,speed);
					motorShield_setSpeed(MOTOR_C,-speed);
					motorShield_setSpeed(MOTOR_D,-speed);
					state = TurnRight;
					break;
	
	case 4:
	
					motorShield_setSpeed(MOTOR_A,speed);
					motorShield_setSpeed(MOTOR_B,-speed);
					motorShield_setSpeed(MOTOR_C,speed);
					motorShield_setSpeed(MOTOR_D,speed);
					state = TurnLeft;
					break;
	}
}

void motorShield_stop(int speed)
{
	motorShield_setBrake(MOTOR_A,speed);
	motorShield_setBrake(MOTOR_B,speed);
	motorShield_setBrake(MOTOR_C,speed);
	motorShield_setBrake(MOTOR_D,speed);
	state = Stopped;
}

void speedUp(int speed)
{
	speed = speed + 10;
	if (speed >= 100)
	{
		speed = 100;
	}
}

void speedDown(int speed)
{
	speed = speed - 10;
	if (speed <= 0)
	{
		speed = 0;
	}
}

void Teleoperation(uint8_t command)
{
	switch(command)
	{
		case 'F': motorShield_move(Forward,actualSpeed);
		break;
		case 'B': motorShield_move(Backward,actualSpeed);
		break;
		case 'R': motorShield_move(TurnRight,actualSpeed);
		break;
		case 'L': motorShield_move(TurnLeft,actualSpeed);
		break;
		case 'S': motorShield_stop(100);
		break;
		case '+': speedUp(actualSpeed);
		break;
		case '-': speedDown(actualSpeed);
		break;
		
	}
}

#endif