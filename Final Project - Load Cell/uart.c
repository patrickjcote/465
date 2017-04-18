#include <msp430.h>
#include "uart.h"



void uartInit(void)
{
	if (CALBC1_1MHZ==0xFF)					// If calibration constant erased
	{
		while(1);                               // do not load, trap CPU!!
	}
	DCOCTL = 0;                             // Select lowest DCOx and MODx settings
	BCSCTL1 = CALBC1_16MHZ;                  // Set DCO
	DCOCTL = CALDCO_16MHZ;
	P1SEL = BIT1 + BIT2 ;                   // P1.1 = RXD, P1.2=TXD
	P1SEL2 = BIT1 + BIT2 ;                  // P1.1 = RXD, P1.2=TXD
	UCA0CTL1 |= UCSSEL_2;                   // SMCLK
	UCA0BR0 = 131;                          // 16MHz 9600
	UCA0BR1 = 6;                            // 16MHz 9600
	UCA0MCTL = UCBRS0;                      // Modulation UCBRSx = 1
	UCA0CTL1 &= ~UCSWRST;                   // **Initialize USCI state machine**
	IE2 |= UCA0RXIE;                        // Enable USCI_A0 RX interrupt

	__bis_SR_register(GIE);       			// Enter LPM0, interrupts enabled
}


void outputMessage(char messagePR[], int lengthPR, int newLine){

	volatile unsigned int i;

	//for length of messagePR[] output that char to the Tx buffer

	for(i = 0; i < lengthPR; i++){
		while (!(IFG2&UCA0TXIFG));          // USCI_A0 TX buffer ready?
		UCA0TXBUF = messagePR[i];
	}//for

	if(newLine){
		while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
		UCA0TXBUF = 0x0A;						// TX -> Line Feed
		while (!(IFG2&UCA0TXIFG));              // USCI_A0 TX buffer ready?
		UCA0TXBUF = 0x0D;						// Tx -> Carriage return
	}

}//outputMessage()


