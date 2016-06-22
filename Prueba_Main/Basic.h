
#ifndef BASIC_H_
#define BASIC_H_

#include <math.h>
#include <stdlib.h>
#include <avr/io.h>

#define F_CPU 16000000L 
#define PI 3.14159265

#define HIGH 0x1
#define LOW  0x0
#define INPUT 0x0
#define OUTPUT 0x1
#define TRUE 0x1
#define FALSE 0x0

//typedef uint8_t BYTE;
//typedef uint8_t BOOL;

#define _BV(bit) (1 << bit)
#define cbi(port,bit) (port) &= ~(1 << (bit))
#define sbi(port,bit) (port) |= (1 << (bit))

//#define min(a,b) (((a)<(b))?(a):(b))
//#define max(a,b) (((a)>(b))?(a):(b))

float min(float a, float b){
	if(a < b) return a;
	else return b;
}
float max(float a, float b){
	if(a > b) return a;
	else return b;
}
#endif /* BASIC_H_ */