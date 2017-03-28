#include <msp430.h> 

/*
 * main.c
 */
#define FREQUENCY 1 //Hz

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;     // Set Clock Speed
	DCOCTL = CALDCO_16MHZ;

	P1DIR = BIT0 + BIT6;
	P1OUT = 0x00;
	volatile int i;

	while(1){

		P1OUT ^= BIT0 + BIT6;
		for(i=0;i<(1000/FREQUENCY); i++){
			__delay_cycles(16000);
		}
	}

	return 0;
}
