#include <msp430.h>

#define SEV_SEG_ADDRESS 	0x70
#define COMMAND_BYTE 		0x00
#define CONTROL_BYTE		0x77		// 0x(Max Brightnes)(Dynamic Mode with all digits)

volatile unsigned char row, col, dispFlag, dispNum, dispNum2;
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
	dispNum2 = 0;

	char txBuf[7];
	char characterMap[16] = {0x7E, 0x30, 0x6D, 0x79, 0x33, 0x5B, 0x5F, 0x70, 0x7F, 0x73, 0x77, 0x1F, 0x4E, 0x3D, 0x4F, 0x47};
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

	i2cInit();								// Initialize i2c
	__delay_cycles(50000);

	txBuf[0] = SEV_SEG_ADDRESS;
	txBuf[1] = COMMAND_BYTE;
	txBuf[2] = CONTROL_BYTE;
	txBuf[3] = characterMap[0];
	txBuf[4] = characterMap[1];
	txBuf[5] = characterMap[2];
	txBuf[6] = characterMap[3];

	i2cWrite(txBuf, 7);


	__bis_SR_register(GIE);

	while(1){

		if(dispFlag){

			txBuf[0] = SEV_SEG_ADDRESS;
			txBuf[1] = COMMAND_BYTE;
			txBuf[2] = CONTROL_BYTE;
			txBuf[6] = txBuf[5];
			txBuf[5] = txBuf[4];
			txBuf[4] = txBuf[3];
			txBuf[3] = characterMap[dispNum];

			__delay_cycles(50000);
			i2cWrite(txBuf, 7);
			__delay_cycles(50000);
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

	debounce++;

	if(debounce>10){
		dispFlag = 1;
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
		debounce = 0;
	}

	__delay_cycles(500);

	P2IFG = 0x00;
}

