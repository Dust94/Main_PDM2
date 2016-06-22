#define F_CPU 16000000L

#include <avr/io.h>
#include "avr/interrupt.h"
#include "uart_michel.h"
#include "Magnetometer_HMC5883L.h"
#include "motorShield.h"

int headingDegrees = 0;
float heading = 0;
int Mag_Raw[3];
float Mag_Scaled[3];

uint8_t dato; //dato recibido por serial
unsigned char flag_Motor = 0;

char hedi[10];

ISR(USART0_RX_vect){
	dato = UDR0;
	if( (dato == 'F') || (dato == 'B') || (dato == 'R') || (dato == 'L') || (dato == 'S') || (dato == '+') || (dato == '-') ){
		flag_Motor = 1;
	}
}

int main(void){
	cli(); //desabilita todas las interrupciones
	uart_init(0); // Para comunicacion Serial con la PC
	i2c_init(); // Para la comunicacion I2C con el IMU
	motorShield_init(); //Para usar el PWM hacia los motores
	
	uart_println(0,"reset"); uart_println(0,"iniciar");
	sei();	//habilita todas las interrupciones
	
	while (1) {
		if(flag_Motor == 1 ) {
			Teleoperation(dato);
			flag_Motor = 0;
		}
		uart_println(0,"dentro main");
		Compass_ReadScaledAxis(&Mag_Scaled[0]);
		heading = atan2(Mag_Scaled[1],Mag_Scaled[0]) + PI;
		headingDegrees = heading*(180/PI);
		uart_println(0,itoa(headingDegrees, hedi, 10));
		
	}//Fin While(1)
}//Fin Main()