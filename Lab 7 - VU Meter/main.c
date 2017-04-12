#include <msp430.h>

#define SEV_SEG_ADDRESS 0x70
#define COMMAND_BYTE 0x00
#define CONTROL_BYTE 0x77 // 0x(Max Brightnes)(Dynamic Mode with all digits)
#define ADC_BUF_SIZE 50
#define GAIN 17

signed int adcBuf[ADC_BUF_SIZE];
volatile int adcBufPtr = 0;

void adcInit(void);

void main(void) {
	WDTCTL = WDTPW | WDTHOLD; // Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;     // Set Clock Speed
	DCOCTL = CALDCO_16MHZ;

	// Variables init
	volatile unsigned int i, average;
	volatile unsigned long averageTemp = 512;


	char txBuf[7];
	char characterMap[2] = {0x00, 0x30};

	//Timer Setup
	TA1CCR0 = 0xFFFF;
	TA1CTL = TASSEL_2 + MC_2; // SMCLK, contmode,
	TA1CCTL0 |= CCIE; // Timer A1 interrupt enable

	i2cInit(); // Initialize i2c
	__delay_cycles(50000);
	adcInit();

	txBuf[0] = SEV_SEG_ADDRESS;
	txBuf[1] = COMMAND_BYTE;
	txBuf[2] = CONTROL_BYTE;
	txBuf[3] = characterMap[1];
	txBuf[4] = characterMap[0];
	txBuf[5] = characterMap[0];
	txBuf[6] = characterMap[0];

	i2cWrite(txBuf, 7);


	__bis_SR_register(GIE);

	while(1){

		ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start

		averageTemp = 0;
		for(i = 0; i < ADC_BUF_SIZE; i++){
			averageTemp += adcBuf[i];
		}

		average = averageTemp/ADC_BUF_SIZE;



		switch(average/254){
		case 0:
			txBuf[3] = characterMap[0];
			txBuf[4] = characterMap[0];
			txBuf[5] = characterMap[0];
			txBuf[6] = characterMap[0];
			break;
		case 1:
			txBuf[3] = characterMap[1];
			txBuf[4] = characterMap[0];
			txBuf[5] = characterMap[0];
			txBuf[6] = characterMap[0];
			break;
		case 2:
			txBuf[3] = characterMap[1];
			txBuf[4] = characterMap[1];
			txBuf[5] = characterMap[0];
			txBuf[6] = characterMap[0];
			break;
		case 3:
			txBuf[3] = characterMap[1];
			txBuf[4] = characterMap[1];
			txBuf[5] = characterMap[1];
			txBuf[6] = characterMap[0];
			break;
		case 4:
			txBuf[3] = characterMap[1];
			txBuf[4] = characterMap[1];
			txBuf[5] = characterMap[1];
			txBuf[6] = characterMap[1];
			break;
		}

		txBuf[0] = SEV_SEG_ADDRESS;
		txBuf[1] = COMMAND_BYTE;
		txBuf[2] = CONTROL_BYTE;


		__delay_cycles(50000);
		i2cWrite(txBuf, 7);
		__delay_cycles(50000);


	}// forever while
}// main()

void adcInit(void){

	P1SEL |= BIT0;
	ADC10CTL1 = INCH_0;
	ADC10CTL0 = SREF_0 + ADC10SHT_3 + ADC10ON + ADC10IE;  // Vcc & Vss as reference, Sample and hold for 64 Clock cycles, ADC on, ADC interrupt enable
	ADC10AE0 |= BIT0;                         // ADC input enable P1.3

}


// Timer A0 Interrupt
#pragma vector=TIMER1_A0_VECTOR
__interrupt void TIMERA11_ISR(void)
{

	ADC10CTL0 |= ENC + ADC10SC; // Sampling and conversion start

}//timerA1 interrupt()


// ADC10 interrupt service routine
#pragma vector=ADC10_VECTOR
__interrupt void ADC10_ISR (void)
{
	adcBuf[adcBufPtr] = (ADC10MEM - 512)*GAIN;
	if(adcBuf[adcBufPtr] < 0 )
		adcBuf[adcBufPtr] = -1*adcBuf[adcBufPtr];
	adcBufPtr++;
	adcBufPtr %= ADC_BUF_SIZE;
}
