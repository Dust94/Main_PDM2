#define F_CPU 16000000L

#include <avr/io.h>
#include <stdlib.h>
#include <math.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include "uart_michel.h"
#include "I2C_Master.h"
#include "Magnetometer_HMC5883L.h"
#include "motorShield.h"
#include "Analog.h"
#include "Bateria.h"
#include "Ultrasonic.h"

int headingDegrees = 0;
float heading = 0;
int Mag_Raw[3];
float Mag_Scaled[3];

uint8_t dato; //dato recibido por serial
unsigned char flag_Motor = 0;

char hedi[10], strBateria[10], str_ul1, str_ul2[10];

ISR(USART0_RX_vect){
	dato = UDR0;
	//while (!( UCSR0A & (1<<UDRE0))); 
	if( (dato == 'F') || (dato == 'B') || (dato == 'R') || (dato == 'L') || (dato == 'S') || (dato == '+') || (dato == '-') ){
		flag_Motor = 1;
	}
}

int main(void){
	cli(); //desabilita todas las interrupciones
	
	analog_init(); //Para los ultrasonidos y la Bateria
	Timer4_init(); // Ultrasonido
	Timer5_init(); // Ultrasonido
	
	uart_init(0); // Para comunicacion Serial con la PC
	i2c_init(); // Para la comunicacion I2C con el IMU
	init_HMC5883L(); // Para el IMU
	motorShield_init(); //Para usar el PWM hacia los motores
	
	uart_println(0,"iniciar - Fuera del While(1)");
	sei();	//habilita todas las interrupciones
	
	while (1) {
		if(flag_Motor == 1 ) {
			Teleoperation(dato);
			uart_print(0,"ingreso dato");
			uart_println(0, dato);
			flag_Motor = 0;
		}
		uart_println(0,"dentro del While(1)");
		
		//Lecturas de Sensores		
		Ultrasonic1 = ultrasonic_read(port1);
		Ultrasonic2 = ultrasonic_read(port2);
		CargaBateria = Bateria();
		Compass_ReadScaledAxis(&Mag_Scaled[0]);
		heading = atan2(Mag_Scaled[1],Mag_Scaled[0]) + PI;
		headingDegrees = heading*(180/PI);
		
		//Impresion de Valores
		uart_print(0, "Ultrasonic-1: ");	//uart_println(0,itoa(Ultrasonic1, str_ul1, 10)); //UltrasonicPrint(Ultrasonic1); 
		uart_print(0, "Ultrasonic-2: ");	//uart_println(0,itoa(Ultrasonic2, str_ul2, 10)); //UltrasonicPrint(Ultrasonic2); 
		uart_print(0, "Carga-Bateria: ");	uart_println(0,itoa(CargaBateria, strBateria, 1)); 
		uart_print(0, "heading:  ");		uart_println(0,itoa(headingDegrees, hedi, 10));
		_delay_ms(10); //Espero un rato para tratar de visualizar algo
		
		
	}//Fin While(1)
}//Fin Main()