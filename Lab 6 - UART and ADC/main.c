#include <msp430.h> 
#include "uart.h"

void adcInit(void);
void num2str(char stringOut[], int num, int k);
void displayAdc(void);
void getPWM(void);
void keypadInit(void);

volatile unsigned char row, col, dispFlag, dispNum, dispNum2, pwm[3];
volatile unsigned int debounce;

volatile unsigned int keypadArray[4][4]={
		{0x01, 0x02, 0x03, 0x0A},
		{0x04, 0x05, 0x06, 0x0B},
		{0x07, 0x08, 0x09, 0x0C},
		{0x0E, 0x00, 0x0F, 0x0D}         };

void main(void) {

	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer

	uartInit();

	// Variables init
	volatile unsigned int i;
	row = 0;
	dispFlag = 0;
	dispNum = 0;
	dispNum2 = 0;

	// PWM Out
	P2DIR |= BIT1;
	P2SEL |= BIT1;
	TA1CTL |= TASSEL_2 + MC_1;
	TA1CCTL1 |= OUTMOD_7;
	TA1CCR1 = 0;
	pwm[0] = 0;
	pwm[1] = 0;
	pwm[2] = 0;
	adcInit();
	keypadInit();

	__bis_SR_register(GIE);

	while(1){

		if(dispFlag){
			if(dispNum == 0x0F){
				while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
				UCA0TXBUF = 0x0A; // TX -> Line Feed
				while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
				UCA0TXBUF = 0x0D; // Tx -> Carriage return
			}
			else{
				dispNum += 0x30;

				while (!(IFG2&UCA0TXIFG));          // USCI_A0 TX buffer ready?
				UCA0TXBUF = dispNum;
			}
			dispFlag = 0;
		}


	}// forever while
}// main()


void adcInit(void){

	P1SEL |= BIT0;
	ADC10CTL1 = INCH_0;
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;  // Vcc & Vss as reference, Sample and hold for 64 Clock cycles, ADC on, ADC interrupt enable
	ADC10AE0 |= BIT0;                         // ADC input enable P1.3

}

void displayAdc(void){
	volatile unsigned int ADC_value=0;
	char output[4];
	__delay_cycles(1000); // Wait for ADC Ref to settle
	ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start
	__bis_SR_register(CPUOFF + GIE); // Low Power Mode 0 with interrupts enabled
	ADC_value = ADC10MEM;

	num2str(output,ADC_value,4);
	outputMessage(output,4,1);
}

void keypadInit(void){

	// Output Init
	P1DIR |= BIT3 + BIT4; // Keypad
	// Keypad Init
	P2REN |= BIT0 + BIT2 + BIT3 + BIT5; // Enable polling resistor
	P2OUT |= BIT0 + BIT2 + BIT3 + BIT5; // Pull up Resistor
	P2IE |= BIT0 + BIT2 + BIT3 + BIT5; // Enable Interrupt
	P2IES |= BIT0 + BIT2 + BIT3 + BIT5; // High/Low interrupt
	P2IFG = 0x00; // Clear Interrupt Flag
	//Timer Setup
	TA1CCR0 = 0xFFFF;
	TA1CTL = TASSEL_2 + MC_2; // SMCLK, contmode,
	TA1CCTL0 |= CCIE; // Timer A1 interrupt enable

}

void num2str(char stringOut[], int num, int k){
	volatile int i, temp, divider = 1;
	for(i=1; i<k; i++)
		divider *= 10;
	for(i=0; i<k; i++){
		temp = num/divider;
		stringOut[i] = temp+0x30;
		num -= temp*divider;
		divider /= 10;
	}
}



// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	__bic_SR_register_on_exit(CPUOFF);        // Return to active mode }
}

#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
	volatile char inChar;
	inChar = UCA0RXBUF; // Set Received char to inCha

	if(pwm[0]){
		if(0x29 < inChar && inChar < 0x3A)
		{
			pwm[pwm[0]] = (inChar - 0x30);
			while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
			UCA0TXBUF = inChar; // TX -> Line Feed
			pwm[0]++;
			if(pwm[0]>2){
				pwm[0] = 0;
				while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
				UCA0TXBUF = 0x0A; // TX -> Line Feed
				while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
				UCA0TXBUF = 0x0D;
				char out[] = "PWM Set to 0x%";
				out[11] = 0x30 + pwm[1];
				out[12] = 0x30 + pwm[2];
				outputMessage(out,14,1);
			}
		}
		else if(pwm[0] = 1){
			pwm[2] = pwm[1];
			pwm[1] = 0;
			pwm[0] = 0;
			while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
			UCA0TXBUF = 0x0A; // TX -> Line Feed
			while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
			UCA0TXBUF = 0x0D;
			char out[] = "PWM Set to 0x%";
			out[11] = 0x30 + pwm[1];
			out[12] = 0x30 + pwm[2];
			outputMessage(out,14,1);
		}
	}
	if(inChar=='Q' || inChar=='q')
		displayAdc();
	if(inChar=='P' || inChar=='p')
		getPWM();

}

void getPWM(){
	char outMessage[] = "Input PWM Duty Cycle (0-99): ";
	outputMessage(outMessage,29,0);
	pwm[0] = 1;
}

// Timer A0 Interrupt
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMERA11_ISR(void)
{
	volatile int bitmask, pwmVal;
	// Keypad signal
	row++;
	row %= 4;
	bitmask = (row << 3) + ((P1OUT & ~BIT3) & ~BIT4);
	P1OUT = bitmask;

	if(pwm[0] == 0){
		pwmVal = pwm[1]*10+pwm[2];
		TA1CCR1 = 0x28F*pwmVal;
	}

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
