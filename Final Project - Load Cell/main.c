#include <msp430.h> 
#include "uart.h"

#define DATA BIT0
#define CLOCK BIT3
#define DELAY 200

void long2str(char *, unsigned long);
unsigned long readHX711(int unsigned);

volatile int readFlag = 0, channelToRead = 1;

void main(void) {

WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer


uartInit();
unsigned long readInValue;

// Variables init
char outputBuffer[8];

// Ports Init
P1DIR |= CLOCK; // Clock Output
P1DIR &= ~DATA; // Data Input
P1IE  |= DATA; // Enable Interrupt
P1IES |= DATA; // High/Low interrupt
P1IFG = 0x00; // Clear Interrupt Flag
P1OUT &= ~CLOCK; // Clock Low



__bis_SR_register(GIE);

while(1){

if(readFlag){

readInValue = readHX711(channelToRead);
long2str(outputBuffer, readInValue);
outputMessage(outputBuffer,8,1);

readFlag = 0; // Clear readFlag
P1IFG = 0x00; // Clear interrupt
P1IE  |= DATA; // Enable Interrupt

}// if readFlag


}// while(1)
}// main()

unsigned long readHX711(int unsigned channel){

// Read the Hx711 24-Bit ADC for Weigh Scales
// Channel Select: Clock Pulses - channel and gain
// 1: Input A, Gain 128
// 2: Input B, Gain 32
// 3: Input A, Gain 64

unsigned long readIn = 0;
volatile unsigned int i;

for(i=0;i<24;i++){ // For 24 Bits
P1OUT |= CLOCK; // Clock Up
__delay_cycles(DELAY); // ~1.25us


readIn <<= 1; // Shift bit
if(P1IN & DATA) // If data is high
readIn++; // Set LSB

P1OUT &= ~CLOCK; // Clock Down
__delay_cycles(DELAY); // ~1.25us

}// for 24 bits

for(i=0;i<channel;i++){
P1OUT |= CLOCK; // Clock Up
__delay_cycles(DELAY); // ~1.25us
P1OUT &= ~CLOCK; // Clock Down
__delay_cycles(DELAY); // ~1.25us
}

readIn ^= 0x800000; // Flip MSB - adc range: [0x800000, 0x7FFFFF]
return readIn;

}

void long2str(char stringOut[], unsigned long num){
volatile unsigned int i;
unsigned long  temp, divider = 1;

for(i=1; i<8; i++)
divider *= 10;

for(i=0; i<9; i++){
temp = num/divider;
stringOut[i] = temp+0x30;
num -= temp*divider;
divider /= 10;
}
}// long2str()


#pragma vector=USCIAB0RX_VECTOR
__interrupt void USCI0RX_ISR(void)
{
volatile char inChar;
inChar = UCA0RXBUF; // Set Received char to inChar
while (!(IFG2&UCA0TXIFG));          // USCI_A0 TX buffer ready?
channelToRead = inChar - 0x30;
UCA0TXBUF = inChar;
}// USCI interrupt()


#pragma vector=PORT1_VECTOR
__interrupt void Port_1(void)
{
readFlag = 1;
P1IE &= ~DATA; // Disable Interrupt
__delay_cycles(100);

}

