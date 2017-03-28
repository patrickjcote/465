#include <msp430.h> 

// Defines
#define PWM_PERIOD 		40000 	// (16MHz*20ms PWM Period)/(Clock Divider)
#define MOTOR_NEUTRAL	3000	// PWM_PERIOD * 7.5% = 1.5ms
#define MOTOR_HIGH		4000	// PWM_PERIOD * 10% = 2ms
#define MOTOR_LOW		2000	// PWM_PERIOD * 5% = 1ms

// Globals
volatile unsigned int row=0, col=0, leftPWM=0, rightPWM=0, robotDirection=0, changeDirFlg=0;
volatile unsigned int keypadArray[4][4]={
		{0x01, 0x02, 0x03, 0x0A},
		{0x04, 0x05, 0x06, 0x0B},
		{0x07, 0x08, 0x09, 0x0C},
		{0x0E, 0x00, 0x0F, 0x0D}  };


void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;     // Set Clock Speed
	DCOCTL = CALDCO_16MHZ;

	//Output Init
	P1DIR = BIT3 + BIT4;	// Output Keypad
	P1OUT = 0x00;
	// PWM Output Init
	P2DIR |= BIT1 + BIT4;
	P2SEL |= BIT1 + BIT4;
	// Keypad Init
	P2REN |= BIT0 + BIT2 + BIT3 + BIT5;		// Enable polling resistor
	P2OUT |= BIT0 + BIT2 + BIT3 + BIT5;		// Pull up Resistor
	P2IE |= BIT0 + BIT2 + BIT3 + BIT5;		// Enable Interrupt
	P2IES |= BIT0 + BIT2 + BIT3 + BIT5;		// High/Low interrupt
	P2IFG = 0x00;							// Clear Interrupt Flag
	// Timer Init
	TA1CCR0 = PWM_PERIOD;
	TA1CTL = TASSEL_2 + MC_1 + ID_3;				// SMCLK, contmode, 2^3 divide
	TA1CCTL0 |= CCIE;						// Timer A1 interrupt enable
	TA1CCTL1 |= OUTMOD_7;
	TA1CCR1 = 0;
	TA1CCTL2 |= OUTMOD_7;
	TA1CCR2 = 0;

	__bis_SR_register(GIE);

	while(1){
		if(changeDirFlg)
		{
			switch(robotDirection){
			case 2:		//Forward
				leftPWM = MOTOR_HIGH;
				rightPWM = MOTOR_LOW;
				break;
			case 8:		//Reverse
				leftPWM = MOTOR_LOW;
				rightPWM = MOTOR_HIGH;
				break;
			case 6:		//Right
				leftPWM = MOTOR_HIGH;
				rightPWM = MOTOR_HIGH;
				break;
			case 4:		//Left
				leftPWM = MOTOR_LOW;
				rightPWM = MOTOR_LOW;
				break;
			default:
				leftPWM = MOTOR_NEUTRAL;
				rightPWM = MOTOR_NEUTRAL;
				break;
			}
			changeDirFlg = 0;
		}
	}// forever while
}// main()


// Timer A0 Interrupt
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMERA10_ISR(void)
{
	volatile int bitmask = 0;

	// Keypad signal
	row++;
	row %= 4;
	bitmask = (row << 3) + ((P1OUT & ~BIT3) & ~BIT4);
	P1OUT = bitmask;

	TA1CCR1 = leftPWM;
	TA1CCR2 = rightPWM;

}//timerA1.0 interrupt()


#pragma vector=PORT2_VECTOR
__interrupt void Port_2(void)
{

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

	robotDirection = keypadArray[row][col];
	changeDirFlg = 1;

	P2IFG = 0x00;
}

