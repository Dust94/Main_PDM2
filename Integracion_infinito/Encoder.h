
#ifndef ENCODER_H_
#define ENCODER_H_

#include <avr/io.h>
#include <avr/interrupt.h>
#include <stdio.h>
#include <stdlib.h>

//const float pi = 3.14;
volatile uint8_t portbhistory = 0xFF;     // default is high because the pull-up
volatile int rpmcount16 = 0;
volatile int rpmcount17 = 0;
volatile int rpmcount18 = 0;
volatile int rpmcount19 = 0;
int rpm16 = 0;
int rpm17 = 0;
int rpm18 = 0;
int rpm19 = 0;
int rpm = 0;
int distanciaEnc = 0;
int auxdistancia = 0;
int auxrpm = 0;

//void ConfiguraTimer2(void);
//void Configura_PCINT(void);

void Configura_PCINT(void)
{
	PCMSK2 |= (1<<PCINT16 | 1<<PCINT17 | 1<<PCINT18 | 1<<PCINT19); // se uilizara el pin PCINT8 9 10 11 para detectar interrupciones por cambio de estados pcint 8 9 10 11
	PCICR |= (1<<PCIE2); // habilito la interrupcion externa por cambio de estado en el grupo en el cual esta el pin PCINT8 9 10 11
	PCIFR = 0b00000000;
}


void Encoder_init(void){
	DDRK &= ~((1 << DDK0) | (1 << DDK1) | (1 << DDK2) | (1<<DDK3)); // Clear the PK0, PK1, PK2, PK3 pin
	// PK0,PK1,PK2, PK3 (PCINT16, PCINT17, PCINT18, PCINT19 pin) are now inputs
	PORTK |= ((1 << 0) | (1 << 1) | (1 << 2) | (1<<3)); // turn On the Pull-up
	// PB0, PB1 and PB2 are now inputs with pull-up enabled
	//ConfiguraTimer2();
	Configura_PCINT();
}

int EncoderDistancia(void)
{
		rpm16 = rpmcount16 / 54.4; //2*19.5
		rpm17 = rpmcount17 / 54.4; //2*34.53 MOTOR NUEVO
		rpm18 = rpmcount18 / 54.4;
		rpm19 = rpmcount19 / 54.4;
		rpm = ((rpm16 + rpm17 + rpm18 + rpm19) / 4);
		
		/*if (auxrpm <= rpm){
			auxrpm = rpm;
		}*/
		//if (distanciaEnc > 100000) distanciaEnc = 0;
		
	return rpm;
}

ISR (PCINT2_vect){
	uint8_t changedbits;

	changedbits = PINK ^ portbhistory;
	portbhistory = PINK;

	if(changedbits & (1 << PK0))
	{
		/* PCINT16 changed */
		rpmcount16 ++;
		//usart_txmensflash(&msgRPM16[0]);
	}

	if(changedbits & (1 << PK1))
	{
		/* PCINT17 changed */
		rpmcount17 ++;
		//usart_txmensflash(&msgRPM17[0]);
	}

	if(changedbits & (1 << PK2))
	{
		/* PCINT18 changed */
		rpmcount18 ++;
		//usart_txmensflash(&msgRPM18[0]);
	}
	
	if (changedbits & (1 << PK3))
	{
		/* PCINT19 changed */
		rpmcount19 ++;
		//usart_txmensflash(&msgRPM19[0]);	
	}
}

#endif /* ENCODER_H_ */