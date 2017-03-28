#include <msp430.h>
#include "lcd.h"

volatile unsigned char row, col, dispFlag, dispNum;
volatile unsigned int debounce;

volatile unsigned int keypadArray[4][4]={
		{0x01, 0x02, 0x03, 0x0A},
		{0x04, 0x05, 0x06, 0x0B},
		{0x07, 0x08, 0x09, 0x0C},
		{0x0E, 0x00, 0x0F, 0x0D}         };


void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;     // Set Clock Speed
	DCOCTL = CALDCO_16MHZ;

	// Variables init
	volatile unsigned int i;
	row = 0;
	dispFlag = 0;
	dispNum = 0;
	debounce = 0;

	// Output Init
	P1DIR |= BIT3 + BIT4;					// Keypad
	// Keypad Init
	P2REN |= BIT0 + BIT2 + BIT3 + BIT5;		// Enable polling resistor
	P2OUT |= BIT0 + BIT2 + BIT3 + BIT5;		// Pull up Resistor
	P2IE |= BIT0 + BIT2 + BIT3 + BIT5;		// Enable Interrupt
	P2IES |= BIT0 + BIT2 + BIT3 + BIT5;		// High/Low interrupt
	P2IFG = 0x00;							// Clear Interrupt Flag
	//Timer Setup
	TA1CCR0 = 0xFFFF;
	TA1CTL = TASSEL_2 + MC_2;				// SMCLK, contmode,
	TA1CCTL0 |= CCIE;						// Timer A1 interrupt enable

	__delay_cycles(200);

	LCD lcd;

	lcdInit(&lcd);

	__bis_SR_register(GIE);

	while(1){
		if(lcd.currentPosition == 0x10){
			lcd.currentPosition = 0x40;
			__delay_cycles(9440);
			lcdComByte(&lcd,0x80+lcd.currentPosition);
		}
		if(lcd.currentPosition == 0x50){
			lcd.currentPosition = 0x00;
			__delay_cycles(9440);
			lcdComByte(&lcd,0x80+lcd.currentPosition);
		}

		if(dispFlag){
			switch(dispNum){
			case 0x0E:					// '*' is pressed
				lcdComByte(&lcd,0x01);	// Clear Screen
				lcd.currentPosition = 0;
				break;
			case 0x0F:					// '#' is pressed
				lcdBackspace(&lcd);
				break;
			default:					// Otherwise display keypressed
				lcdDataByte(&lcd,(dispNum+0x30));
				lcd.currentPosition++;
				break;
			}
			dispFlag = 0;
		}

	}// forever while
}// main()





// Timer A0 Interrupt
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMERA11_ISR(void)
{
	volatile int bitmask;
	// Keypad signal
	row++;
	row %= 4;
	bitmask = (row << 3) + ((P1OUT & ~BIT3) & ~BIT4);
	P1OUT = bitmask;

}//timerA1 interrupt()

#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{
	if(!dispFlag){
		switch(P2IFG){
		case BIT5:
			col = 3;
			break;
		case BIT3:
			col = 2;
			break;
		case BIT2:
			col = 1;
			break;
		case BIT0:
			col = 0;
			break;
		default:
			break;
		}
		dispNum = keypadArray[row][col];
	}

	debounce++;

	if(debounce>5){
		dispFlag = 1;
		debounce = 0;
	}


	__delay_cycles(50);

	P2IFG = 0x00;

}

