#ifndef I2C_MASTER_H_
#define I2C_MASTER_H_

#include "Basic.h" // F_CPU 16000000L 

#define SCL_CLOCK  100000L //Velocidad Standar  100 kHz clock
#define SDA_1 4 //PC4 A4
#define SCL_1 5 //PC5 A5
#define SDA_Mega 1 //PD1 20
#define SCL_Mega 0 //PD0 21

/* Master */
#define TW_START        0x08
#define TW_REP_START          0x10
/* Master Transmitter */
#define TW_MT_SLA_ACK         0x18
#define TW_MT_SLA_NACK        0x20
#define TW_MT_DATA_ACK        0x28
#define TW_MT_DATA_NACK       0x30
#define TW_MT_ARB_LOST        0x38
/* Master Receiver */
#define TW_MR_ARB_LOST        0x38
#define TW_MR_SLA_ACK         0x40
#define TW_MR_SLA_NACK        0x48
#define TW_MR_DATA_ACK        0x50
#define TW_MR_DATA_NACK       0x58

#define TW_STATUS_MASK        (_BV(TWS7)|_BV(TWS6)|_BV(TWS5)|_BV(TWS4)|_BV(TWS3))
#define TW_STATUS       (TWSR & TW_STATUS_MASK)

void i2c_init(void){
  PORTC|= ((1<<SDA_Mega)|(1<<SCL_Mega));  //activa resistencias pull upp para SCL y SDA
  TWSR = 0; //Preescalador = 1 
  TWBR = ((F_CPU/SCL_CLOCK)-16)/2; // Velocidad de 100 kHz clock, fosc_CPU=F_CPU, Preescalador=1
  //TWCR |= (1<<TWIE); //TWI Enable Interrupt Habilitado
}

/*************************************************************************	
  Issues a start condition and sends address and transfer direction.
  return 0 = device accessible, 1 = failed to access device
*************************************************************************/
unsigned char i2c_start(unsigned char address){
    uint8_t twst;
	TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // send START condition
	while(!(TWCR & (1<<TWINT))); // wait until transmission completed
	
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits.
	if ( (twst != TW_START) && (twst != TW_REP_START)) return 1;
	
	// send device address
	TWDR = address;
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT))); // wail until transmission completed and ACK/NACK has been received
	
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits.
	if ( (twst != TW_MT_SLA_ACK) && (twst != TW_MR_SLA_ACK) ) return 1;
	return 0;
}

/*************************************************************************
 Issues a start condition and sends address and transfer direction.
 If device is busy, use ack polling to wait until device is ready
 
 Input:   address and transfer direction of I2C device
*************************************************************************/
void i2c_start_wait(unsigned char address){
    uint8_t twst;
    while ( 1 ){
	    TWCR = (1<<TWINT) | (1<<TWSTA) | (1<<TWEN); // send START condition
    	while(!(TWCR & (1<<TWINT))); // wait until transmission completed
    	
    	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits.
    	if ( (twst != TW_START) && (twst != TW_REP_START)) continue;
    	// send device address
    	TWDR = address;
    	TWCR = (1<<TWINT) | (1<<TWEN);   
    	while(!(TWCR & (1<<TWINT))); // wail until transmission completed
    
    	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits.
    	if ( (twst == TW_MT_SLA_NACK )||(twst ==TW_MR_DATA_NACK) ){    	    
    	    /* device busy, send stop condition to terminate write operation */
	        TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO);	        	        
	        while(TWCR & (1<<TWSTO)); // wait until stop condition is executed and bus released	        
    	    continue;
    	} //Fin If
    	break;
     } //Fin While
} //Fin i2c_start_wait(address)

void i2c_stop(void){ // Terminates the data transfer and releases the I2C bus
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWSTO); //send stop condition
	while(TWCR & (1<<TWSTO)); // wait until stop condition is executed and bus released
}



/*Send one byte to I2C device 
  Input:    byte to be transfered
  Return:   0 write successful 
            1 write failed */
unsigned char i2c_write( unsigned char data ){	
    uint8_t   twst;    
	// send data to the previously addressed device
	TWDR = data;
	TWCR = (1<<TWINT) | (1<<TWEN);
	
	while(!(TWCR & (1<<TWINT))); // wait until transmission completed	
	twst = TW_STATUS & 0xF8; // check value of TWI Status Register. Mask prescaler bits
	if( twst != TW_MT_DATA_ACK) return 1;
	return 0;
}

void TWI_write_data(unsigned char data){
	TWDR=data;  // put data in TWDR
	TWCR=(1<<TWINT)|(1<<TWEN);    // Clear TWI interrupt flag,Enable TWI
	while (!(TWCR & (1<<TWINT))); // Wait till complete TWDR byte transmitted
	while((TWSR & 0xF8) != 0x28); // Check for the acknowledgement
}


/*Read one byte from the I2C device, request more data from device 
  Return:  byte read from I2C device */
unsigned char i2c_readAck(void){
	TWCR = (1<<TWINT) | (1<<TWEN) | (1<<TWEA);
	while(!(TWCR & (1<<TWINT)));    
    return TWDR;
}
/* Read one byte from the I2C device, read is followed by a stop condition 
   Return:  byte read from I2C device */
unsigned char i2c_readNak(void){
	TWCR = (1<<TWINT) | (1<<TWEN);
	while(!(TWCR & (1<<TWINT)));
    return TWDR;
}



unsigned char TWI_read_data(void){
	TWCR=(1<<TWINT)|(1<<TWEN);    // Clear TWI interrupt flag,Enable TWI
	while (!(TWCR & (1<<TWINT))); // Wait till complete TWDR byte transmitted
	while((TWSR & 0xF8) != 0x58); // Check for the acknowledgement
	return TWDR;
}


#endif /* I2C_MASTER_H_ */