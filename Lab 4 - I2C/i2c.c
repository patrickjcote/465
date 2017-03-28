/*	i2c.c
 *
 * 	Author: Patrick Cote
 * 	2/14/2017
 *
 * 	First element in the tx and rx buffer is the 8-bit address of the slave
 *
 *
 */
#include <msp430.h>

#define SDA			BIT1
#define SCL			BIT4
#define I2C_PxDIR	P2DIR
#define I2C_PxIN	P2IN
#define I2C_PxOUT	P2OUT
#define I2C_PERIOD	2000		//

void i2cInit(){

	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;			// Set Clock Speed
	DCOCTL = CALDCO_16MHZ;

	I2C_PxDIR |= SDA + SCL;			// Data direction: Write
	I2C_PxOUT |= SDA + SCL;
}


int i2cWrite(char *txBuf, int txBytes){

	volatile int bitNdx, byteNdx;

	I2C_PxDIR |= SDA + SCL;			// Data direction: Write
	I2C_PxOUT |= SDA + SCL;

	__delay_cycles(I2C_PERIOD);
	// Start Condition
	I2C_PxOUT &= ~SDA;				// Data Low
	__delay_cycles(I2C_PERIOD);
	I2C_PxOUT &= ~SCL;				// Clock Low

	// Write
	for(byteNdx = 0; byteNdx < txBytes; byteNdx++){
		for(bitNdx = 0; bitNdx < 8; bitNdx++){
			__delay_cycles(I2C_PERIOD);
			if(txBuf[byteNdx] & (0x80>>bitNdx))
				I2C_PxOUT |= SDA;
			else
				I2C_PxOUT &= ~SDA;
			__delay_cycles(I2C_PERIOD);

			I2C_PxOUT |= SCL;		// Clock High
			__delay_cycles(I2C_PERIOD);
			I2C_PxOUT &= ~SCL;		// Clock Low

		}//for each bit in txBuf[byteNdx]
		I2C_PxOUT &= ~SDA;
		I2C_PxDIR &= ~SDA;			// Data direction: Read
		__delay_cycles(I2C_PERIOD);

		I2C_PxOUT |= SCL;			// Clock High
		__delay_cycles(I2C_PERIOD);

		if(I2C_PxIN & SDA){			// If Data is High: No Acknowledge
			I2C_PxDIR |= SDA;		// Data direction: Write
			I2C_PxOUT &= ~(SDA+SCL);// Data and Clock Low
			I2C_PxOUT |= SCL;		// // Stop Condition: Clock High
			__delay_cycles(I2C_PERIOD);
			I2C_PxOUT |= SDA;		// Data High
			return -1;				// Return error
		}

		I2C_PxOUT &= ~SCL;			// Clock Low
		I2C_PxDIR |= SDA;			// Data direction: Write
		I2C_PxOUT |= SDA;			// Data High

	}//for each byte in txBuf[]

	// Stop Condition
	I2C_PxOUT |= SCL;				// Clock High
	__delay_cycles(I2C_PERIOD);
	I2C_PxOUT |= SDA;				// Data High

	return txBytes;

}//i2cWrite()

int i2cRead(char *rxBuf, int rxBytes){

	volatile int bitNdx, byteNdx;

	I2C_PxDIR |= SDA + SCL;	// Data direction: Write
	I2C_PxOUT |= SDA + SCL;

	// Start Condition
	I2C_PxOUT &= ~SDA;
	__delay_cycles(I2C_PERIOD);
	I2C_PxOUT &= ~SCL;

	// Write address
	rxBuf[0]++;
	for(bitNdx = 0; bitNdx < 8; bitNdx++){
		__delay_cycles(I2C_PERIOD);
		if(rxBuf[0] & (0x80>>bitNdx))
			I2C_PxOUT |= SDA;		// Data High
		else
			I2C_PxOUT &= ~SDA;		// Data Low
		__delay_cycles(I2C_PERIOD);

		I2C_PxOUT |= SCL;			// Clock High
		__delay_cycles(I2C_PERIOD);
		I2C_PxOUT &= ~SCL;			// Clock Low

	}//for

	I2C_PxDIR &= ~SDA;				// Data direction: Read
	__delay_cycles(I2C_PERIOD);

	I2C_PxOUT |= SCL;				// Clock High
	__delay_cycles(I2C_PERIOD);

	if(I2C_PxIN & SDA){
		return -1;					// No Acknowledge, return error
	}

	I2C_PxOUT &= ~SCL;				// Clock Low

	// Read
	for(byteNdx = 0; byteNdx < rxBytes; byteNdx++){
		rxBuf[byteNdx] = 0;			// Make sure buffer is clear
		for(bitNdx = 0; bitNdx < 8; bitNdx++){
			__delay_cycles(I2C_PERIOD);
			I2C_PxOUT |= SCL;		// Clock High
			if(I2C_PxIN & SDA)
				rxBuf[byteNdx]++;	// Data High
			rxBuf[byteNdx] <<= 1;
			__delay_cycles(I2C_PERIOD);
			I2C_PxOUT &= ~SCL;		// Clock Low

		}//for each bit in rxBuf[byteNdx]

		I2C_PxDIR &= ~SDA;			// Data direction: Read
		__delay_cycles(I2C_PERIOD);

		I2C_PxOUT |= SCL;			// Clock High
		__delay_cycles(I2C_PERIOD);

		if(I2C_PxIN & SDA){
			return -1;				// No Acknowledge, return error
		}

		I2C_PxOUT &= ~SCL;			// Clock Low
		I2C_PxDIR |= SDA;			// Data direction: Write
		I2C_PxOUT |= SDA;			// Data High

	}//for each byte in txBuf[]

	return rxBytes;

}

