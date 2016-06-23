/*
 * Bateria.h
 *
 * Created: 29/05/2016 09:36:23 a.m.
 *  Author: SARA JULIA
 */ 


#ifndef BATERIA_H_
#define BATERIA_H_

#include <avr/io.h>
#include <util/delay.h>
#include "Analog.h"

int Bateria(void);

int CargaBateria;

unsigned int suma = 0;
uint8_t puertoBateria = 13;
//unsigned int voltProm = 0;

int Bateria(){
	
	for (int i = 0; i<16; i++)
	{
		suma += analog_read(puertoBateria);
		_delay_us(100);
	}
	CargaBateria = suma/16;
	suma=0;

	//Valor entre 0 y 1023, correspondiente a 0-5v, que a su vez estan escalados de 12v
	

	return CargaBateria;
}

#endif /* BATERIA_H_ */

