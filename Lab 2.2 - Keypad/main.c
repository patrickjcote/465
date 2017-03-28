 #include <msp430.h>

/*
 * main.c
 */
#define FREQUENCY 2 // (1/2)Hz
#define RED_LED BIT0
#define GREEN_LED BIT6

void flashLights();

volatile unsigned int timer, row, col, dispFlag, dispNum, clockEdge, dispEdge;

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
	volatile unsigned int dispPos, dispMask,
	timer = 0;
	row = 0;
	dispFlag = 0;
	dispNum = 0;
	dispPos = 0;
	clockEdge = 0;
	dispEdge = 0;

	//Output Init
	P1DIR = BIT0 + BIT3 + BIT4 + BIT6;	// Output LEDS and Keypad
	P1OUT = 0x00;
	//	//Keypad Init
	P2REN |= BIT0 + BIT2 + BIT3 + BIT5;		// Enable polling resistor
	P2OUT |= BIT0 + BIT2 + BIT3 + BIT5;		// Pull up Resistor
	P2IE |= BIT0 + BIT2 + BIT3 + BIT5;		// Enable Interrupt
	P2IES |= BIT0 + BIT2 + BIT3 + BIT5;		// High/Low interrupt
	P2IFG = 0x00;							// Clear Interrupt Flag
	//Timer Setup
	TA1CCR0 = 0xFFFF;
	TA1CTL = TASSEL_2 + MC_2;				// SMCLK, contmode,
	TA1CCTL0 |= CCIE;						// Timer A1 interrupt enable

	__bis_SR_register(GIE);

	while(1){

		if(clockEdge){
			if(dispFlag){

				dispMask = (dispNum >> (dispPos)) & 0x01;
				if(dispMask)
					P1OUT |= GREEN_LED;
				else
					P1OUT &= ~GREEN_LED;

				dispPos++;

				if(dispPos>3){
					dispNum = 0;
					dispFlag = 0;
					dispPos = 0;
					dispEdge = 1;

				}// if whole number has been displayed
			}// display number

			clockEdge = 0;

		}//clock edge
	}// forever while
}// main()


// Timer A0 Interrupt
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMERA11_ISR(void)
{
	volatile int bitmask = 0;

	timer++;

	// Clock signal
	if(timer > (480/FREQUENCY/2)){
		if(dispEdge){
			P1OUT &= ~(GREEN_LED + RED_LED);
			dispEdge++;
			dispEdge %= 6;
		}
		else{
			P1OUT ^= RED_LED;
			if(P1OUT & RED_LED)
				clockEdge = 1;
		}

		timer = 0;
	}

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
		dispFlag = 1;
		dispEdge = 1;
	}
	P2IFG = 0x00;
}

