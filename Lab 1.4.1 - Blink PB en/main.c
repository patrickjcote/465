#include <msp430.h> 

#define FREQUENCY 15 //Hz

void delay();

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;     // Set Clock Speed
	DCOCTL = CALDCO_16MHZ;

	//Output INIT
	P1DIR = BIT0 + BIT6;	// Output LEDS
	P1OUT = BIT0;
	//Button INIT
	P1REN |= BIT3;		// Enable polling resistor
	P1OUT |= BIT3;		// Pull up Resistor

	while(1){

		if(!(P1IN & BIT3)){
			P1OUT ^= BIT0 + BIT6;
			delay();
		}// if button pressed

	}// main while

	return 0;
}


void delay(){
	volatile int i;
	for(i=0;i<(1000/FREQUENCY/2); i++){
		__delay_cycles(16000);
	}
}
