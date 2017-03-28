#include <msp430.h> 

// Defines
#define PWM_PERIOD 16000000/(8*50)   // 16MHz/(Clock Divider*Frequency)  31Hz>

// Globals
volatile unsigned int row=0, col=0, lcdBrightness=5;



void main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;     // Set Clock Speed
	DCOCTL = CALDCO_16MHZ;


	// PWM Output Init
	P2DIR |= BIT1;
	P2SEL |= BIT1;
	// Timer Init
	TA1CCR0 = PWM_PERIOD;
	TA1CTL = TASSEL_2 + MC_1 + ID_3;	// SMCLK, Up to CCR0 Mode, 2^3 divide
	TA1CCTL0 = CCIE;
	TA1CCTL1 |= OUTMOD_7;
	TA1CCR1 = 0;

	__bis_SR_register(GIE);

	while(1){

		lcdBrightness = PWM_PERIOD / 10;

	}// forever while
}// main()



#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMERA10_ISR(void)
{

	TA1CCR1 = lcdBrightness;

}// timer A1

