#include <msp430.h> 

/*
 * main.c
 */
#define FREQUENCY 5 //Hz

void display(int);
void delay();

volatile int hitFlag, randFlag;

int main(void) {
	WDTCTL = WDTPW | WDTHOLD;	// Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;     // Set Clock Speed
	DCOCTL = CALDCO_16MHZ;

	//Output INIT
	P1DIR = BIT0 + BIT6;	// Output LEDS
	P1OUT = 0x00;
	//Button INIT
	P1REN |= BIT3;		// Enable polling resistor
	P1OUT |= BIT3;		// Pull up Resistor
	P1IE = BIT3;		// Enable Interrupt
	P1IES = BIT3;		// High/Low interrupt
	//Timer Setup
	CCR0 = 0xFFFF;
	TACTL = TASSEL_2 + MC_2;                  // SMCLK, contmode

	__bis_SR_register(GIE);

	volatile int i, state;

	hitFlag = 0;
	randFlag = 0;
	state = 0;

	while(1){

		if(randFlag){
			hitFlag = 1;
			display(TA0R%4); // Display "Random" number based on Clock
			__delay_cycles(2000);
			while(hitFlag){}
			randFlag = 0;
		}
		else{
			state++;
			state %= 4;
			display(state);
			delay();
		}


	}

	return 0;
}


void display(int number){

	switch(number){
	case 0:
		P1OUT = BIT3;
		break;
	case 1:
		P1OUT = BIT3 + BIT6;
		break;
	case 2:
		P1OUT = BIT3 + BIT0;
		break;
	case 3:
		P1OUT = BIT3 + BIT0 + BIT6;
		break;
	default:
		break;
	}
}

void delay(){
	volatile int i,j;
	for(i=0;i<(1000/FREQUENCY/2); i++){
		__delay_cycles(16000);
	}
}

#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
	if(P1IFG & BIT3){
		randFlag = 1;
		hitFlag = 0;
		__delay_cycles(0xFFFF);
	}

	P1IFG = 0x00;
}
