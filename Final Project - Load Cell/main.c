#include <msp430.h> 
#include "uart.h"

#define DATA BIT0
#define CLOCK BIT3

void long2str(char *, unsigned long);
unsigned long readHX711(int unsigned);

volatile char readFlag = 0;

void main(void) {

	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer


	uartInit();

	// Variables init
	char outputBuffer[8];

	// Ports Init
	P1DIR &= ~CLOCK;
	P1DIR |= DATA;
	P1REN |= DATA;		// Enable polling resistor
	P1OUT |= DATA;		// Pull up Resistor
	P1IE  |= DATA;		// Enable Interrupt
	P1IES |= DATA;		// High/Low interrupt
	P1IFG = 0x00;		// Clear Interrupt Flag



	__bis_SR_register(GIE);

	while(1){

		if(readFlag){

			long2str(outputBuffer,readHX711(0));
			outputMessage(outputBuffer,8,1);

			readFlag = 0;	// Clear readFlag
			P1IFG = 0x00;	// Clear interrupt

		}// if readFlag


	}// while(1)
}// main()

unsigned long readHX711(int unsigned channel){

	// Read the Hx711 24-Bit ADC for Weigh Scales
	// Channel Select: Clock Pulses - channel and gain
	// 0: 25 - Input A, Gain 128
	// 1: 26 - Input B, Gain 32
	// 2: 27 - Input A, Gain 64

	const unsigned int clockPulse[3] = {25,26,27};
	unsigned long readIn = 0;
	volatile unsigned int i;

	for(i=0;i<clockPulse[channel];i++){			// Init Read Sequence (clock pulses)
		P1OUT |= CLOCK;			// Clock Up
		__delay_cycles(200); 	// ~1.25us
		P1OUT &= ~CLOCK;		// Clock Down
		__delay_cycles(200);	// ~1.25us
	}// for clock pulses

	__delay_cycles(200);		// Delay between Init/Read

	for(i=0;i<24;i++){			// For 24 Bits
		P1OUT |= CLOCK;			// Clock Up
		__delay_cycles(200); 	// ~1.25us
		P1OUT &= ~CLOCK;		// Clock Down
		__delay_cycles(200);	// ~1.25us
		readIn <<= 1;			// Shift bit
		if(P1IN & DATA)			// If data is high
			readIn++;			// Set LSB
	}// for 24 bits

	readIn ^= 0x800000;			// Flip MSB - adc range: [0x800000, 0x7FFFFF]

	return readIn;				// Total read: ~1.3ms
}

void long2str(char stringOut[], unsigned long num){
	volatile unsigned int i, temp, divider = 1;

	for(i=1; i<9; i++)
		divider *= 10;

	for(i=1; i<9; i++){
		temp = num/divider;
		stringOut[i-1] = temp+0x30;
		num -= temp*divider;
		divider /= 10;
	}
}// long2str()


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	volatile char inChar;
	inChar = UCA0RXBUF; // Set Received char to inChar
}// USCI interrupt()


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	readFlag = 1;
	__delay_cycles(100);

}
