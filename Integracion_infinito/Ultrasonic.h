
#ifndef ULTRASONIC_H_
#define ULTRASONIC_H_

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>

#include <stdlib.h>

//#include "Uart.h"
#include "uart_michel.h" 
#include "Analog.h"

//#define Baudios 57600
#define port1 14
#define port2 15

unsigned int Ultrasonic1=0;
unsigned int Ultrasonic2=0;

//const unsigned char NL[] PROGMEM = "\r\n";
//const unsigned char MessageCM[] PROGMEM = " cm";

void Timer4_init(void)
{
	// fosc=16Mhz; Preescalamiento 1:1024; Modo CTC
	
	TCCR4A = (0<<COM4A1 | 0<<COM4A0 | 0<<WGM41 | 0<<WGM40);
	TCCR4B = (0<<WGM43 | 1<<WGM42 | 1<<CS42 | 0<<CS41 | 1<<CS40);
	
	// cálculo de OCR1A:
	// (1/16us)x(1024)x(OCR1A + 1) = 100 ms ---> OCR1A = 1562
	OCR4A = 1562*6;
	TIMSK4 = (1<<OCIE4A);
}

void Timer5_init(void)
{
	// fosc=16Mhz; Preescalamiento 1:1024; Modo CTC
	
	TCCR5A = (0<<COM5A1 | 0<<COM5A0 | 0<<WGM51 | 0<<WGM50);
	TCCR5B = (0<<WGM53 | 1<<WGM52 | 1<<CS52 | 0<<CS51 | 1<<CS50);
	
	// cálculo de OCR1A:
	// (1/16us)x(1024)x(OCR1A + 1) = 100 ms ---> OCR1A = 1562
	
	OCR5A = 1562*10;

	TIMSK5 = (1<<OCIE5A);
}


unsigned int ultrasonic_read(uint8_t port)
{
	unsigned int sum = 0;
	unsigned int measureValue = 0;
	unsigned int samples = 16;
	
	for (int i=0; i < samples;i++)
	{
		sum += analog_read(port)/2;
		_delay_us(100);
	}
	
	measureValue  = sum>>4;
	
	return measureValue;
}

void UltrasonicPrint(unsigned int dato)
{
	unsigned char string[1];
	if (dato>100)  //el rango de valores es de 30cm a 500
	{
		string[0]=dato/100 + 48;
		string[1]=(dato/10)/10 + 48;
		string[2]=(dato/10)%10 + 48;
	}
	else
	{
		string[0]='0';
		string[1]=dato/10 + 48;
		string[2]=dato%10 + 48;
	}
	uart_print(0,&string[0]);
	//Serial_write(&string[0]);
	//uart_print(0," ");
	//Serial_print(&MessageCM[0]);
	uart_println(0, " cm");
	//uart_print(0, &NL[0]);
	//Serial_print(&NL[0]);
}


#endif
