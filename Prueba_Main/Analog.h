
#ifndef ANALOG_H_
#define ANALOG_H_ 

#include <avr/io.h>

void analog_init();
unsigned int analog_read(uint8_t channel);


void analog_init(void)
{
	ADMUX=0x40;
	ADCSRA=0x87;
}

unsigned int  analog_read(uint8_t channel)
{
	ADMUX=(ADMUX & 0xF8)|(channel & 0x07); 
	ADCSRA|=0x40; 
	ADCSRB=0X08;
	while(ADCSRA & 0x40);
	return ADC;
	
}


#endif