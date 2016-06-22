/*
 * Integracion_infinito.c
 *
 * Created: 20/06/2016 11:17:25 p.m.
 * Author : SARA JULIA
 */ 

#define F_CPU 16000000UL

#include <util/delay.h>

#include "Basic.h"
#include "Magnetometer_HMC5883L.h"
#include "Encoder.h"
#include "Analog.h"
//#include "Uart.h"
#include "uart_michel.h"
#include "motorShield.h"
#include "Ultrasonic.h"
#include "Bateria.h"

int headingDegrees = 0;
int Mag_Raw[3];
float Mag_Scaled[3];
int Acce_Raw[3];
int distancia = 0;
uint8_t dato;

char flag=0;
unsigned char flag_tiempo = 0, flag_imprimir = 0;
float heading;
//int Xmain;
//int Ymain; 

char hedi[10];
char string[10];
char strDist[10];
char strFlagImprimir[10];
//unsigned char camera[10];

//char stringx[10];
//char stringy[10];*/

int aux = 0;

//ISR (TIMER4_COMPA_vect) // Entra cada 0.6s
//{
	//uart_println(0,"Cada 0.6 segundos");
	//Ultrasonic1 = ultrasonic_read(port1);
	//uart_print(0,"Ultrasonic-1: ");
	//UltrasonicPrint(Ultrasonic1);

	//Ultrasonic2 = ultrasonic_read(port2);
	//uart_print(0, "Ultrasonic-2: ");
	//UltrasonicPrint(Ultrasonic2);

	/*if (Ultrasonic1<40)
	{
		flag=1;
	}*/
	
	//CargaBateria = Bateria();
	//uart_print(0, "Carga-Bateria: ");
	//uart_print(0,itoa(CargaBateria, string, 10));
	//uart_print(0," ");
	//uart_print(0,"\r\n");

	/*if (dato == 'F')
	{
		aux = EncoderDistancia();
	}else{aux = 0;}*/

	//uart_print(0,"Distancia: ");
	//uart_print(0,itoa(distancia, strDist, 10));
	//uart_print(0," ");
	//uart_print(0,"\r\n");

	/*Compass_ReadScaledAxis(&Mag_Scaled[0]);
	heading = atan2(Mag_Scaled[1],Mag_Scaled[0]) + PI;
	headingDegrees = heading*(180/PI);*/

	/*uart_print(0, "heading: ");
	uart_println(0,itoa(headingDegrees, hedi, 10));
	uart_print(0,"\r\n");
	_delay_us(100);*/
	
	/*Serial1_read(&camera[0]);
	uart_print(0,"Camera: "); uart_println(0,&camera[0]);	
	_delay_us(100);*/
	 //flag_tiempo++;
	 //flag_imprimir++;
	 //uart_println(0,itoa(flag_imprimir,strFlagImprimir, 10));
	 //if(flag_tiempo == 2){
		//flag_tiempo = 0;
		//distancia = distancia + aux*15*PI/60;
		//rpmcount16=0;
		//rpmcount17=0;
		//rpmcount18=0;
		//rpmcount19=0;
	 //}
//}

/*ISR (TIMER5_COMPA_vect) //cada 1s
{	
	distancia = distancia + aux*15*PI/60;
		rpmcount16=0;
		rpmcount17=0;
		rpmcount18=0;
		rpmcount19=0;
}*/

int main(void){	
	cli();
	actualSpeed = 50;

	analog_init();
	Encoder_init();
	//Serial_begin(Baudios);
	//Serial1_begin(Baudios);
	uart_init(0); //Serial de Michel
	DDRL|=0x02;
	PORTL=0x00;	
	Timer4_init();
	Timer5_init();

	motorShield_init();

	//i2c_init();
	//init_HMC5883L();
	sei();
	motorShield_stop(0);
	uart_println(0,"reset");
	uart_println(0,"iniciar");
	while (1)
	{	
		/*if (flag==1)
		{
			//PORTL=0x02;
			_delay_ms(500);
			PORTL=0x00;
			flag=0;
		}*/
		if(uart_available(0)>0){
		
		uart_print(0,"holi antes"); // _delay_ms(1);
		while(uart_available(0)){}
		dato = uart_read(0);
		
		uart_println(0,"     holi despues"); 

		Ultrasonic2 = ultrasonic_read(port2);
		Ultrasonic1 = ultrasonic_read(port1);
		CargaBateria = Bateria();
		if (dato == 'F') aux = EncoderDistancia();
		else aux = 0;
		Compass_ReadScaledAxis(&Mag_Scaled[0]);
		heading = atan2(Mag_Scaled[1],Mag_Scaled[0]) + PI;
		headingDegrees = heading*(180/PI);

		
		//_delay_ms(1);
		//if(flag_imprimir == 4 ){
			//flag_imprimir = 0;
			uart_print(0,"Ultrasonic-1: ");	//UltrasonicPrint(Ultrasonic1); _delay_ms(1);
			uart_print(0,"Ultrasonic-2: ");	//UltrasonicPrint(Ultrasonic2); _delay_ms(1);
			uart_print(0, "Carga-Bateria: "); uart_print(0,itoa(CargaBateria, string, 1)); //_delay_ms(1);
			uart_print(0," "); uart_print(0,"\r\n"); // _delay_ms(1);
			uart_print(0,"Distancia: "); uart_print(0,itoa(distancia, strDist, 10)); //_delay_ms(1);
			uart_print(0," "); uart_print(0,"\r\n");// _delay_ms(10);
			uart_print(0, "heading: "); uart_println(0,itoa(headingDegrees, hedi, 10)); //_delay_ms(1);
			uart_print(0,"\n");
		//}
		Teleoperation(dato);
		if(dato == 'F')
		{
			PCICR |= (1<<PCIE2); // habilito la interrupcion externa por cambio de estado en el grupo en el cual esta el pin PCINT8 9 10 11
			_delay_us(5000);
		}
		else{
			PCICR |= (0<<PCIE2); // Deshabilito la interrupcion externa por cambio de estado en el grupo en el cual esta el pin PCINT8 9 10 11
			_delay_us(5000);
		}
		}
		//_delay_ms(1); //Wait until end last transmission
	}// FIN WHILE
} //FIN MAIN
