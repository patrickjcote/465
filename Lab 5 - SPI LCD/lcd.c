#include <msp430.h>
#include "lcd.h"



void lcdInit(LCD *lcd){

	WDTCTL = WDTPW | WDTHOLD;		// Stop watchdog timer
	BCSCTL1 = CALBC1_16MHZ;			// Set Clock Speed
	DCOCTL = CALDCO_16MHZ;

	SPI_PxDIR |= MOSI + SCL;		// Data direction: Write
	SPI_PxOUT |= MOSI + SCL;

	volatile unsigned int i,j;
	LCD_PxDIR |= LCD_CS + LCD_RS;
	LCD_PxOUT |= LCD_CS + LCD_RS;
	__delay_cycles(50);
	lcdComByte(lcd,0x30);
	__delay_cycles(50);
	char initBytes[11] = {0x30,0x30,0x39,0x14,0x56,0x6D,0x73,0x0C,0x06,0x0F,0x01};
	for(i=0;i<11;i++)
		lcdComByte(lcd,initBytes[i]);

	lcd->currentPosition = 0;
	for(i=2;i>0;i--)
		for(j=16;j>0;j--)
			lcd->memory[i][j] = ' ';
}


void lcdComByte(LCD *lcd, char txChar){

	volatile int bitNdx;
	LCD_PxOUT &= ~LCD_CS;	// Chip Select Low
	LCD_PxOUT &= ~LCD_RS;	// Register Select Low   A0=Command

	for(bitNdx = 0; bitNdx < 8; bitNdx++){
		if(txChar & (0x80>>bitNdx))
			SPI_PxOUT |= MOSI;
		else
			SPI_PxOUT &= ~MOSI;
		__delay_cycles(50);

		SPI_PxOUT &= ~SCL;		// Clock Low
		__delay_cycles(50);
		SPI_PxOUT |= SCL;		// Clock High
		__delay_cycles(50);
		SPI_PxOUT &= ~SCL;		// Clock Low

	}//for each bit in txChar

	LCD_PxOUT |= LCD_CS;	// Chip Select Low

}

void lcdDataByte(LCD *lcd, char txChar){

	volatile int bitNdx;
	LCD_PxOUT &= ~LCD_CS;	// Chip Select Low
	LCD_PxOUT |= LCD_RS;	// Register Select High   A0=Data

	for(bitNdx = 0; bitNdx < 8; bitNdx++){
		if(txChar & (0x80>>bitNdx))
			SPI_PxOUT |= MOSI;
		else
			SPI_PxOUT &= ~MOSI;
		__delay_cycles(50);

		SPI_PxOUT &= ~SCL;		// Clock Low
		__delay_cycles(50);
		SPI_PxOUT |= SCL;		// Clock High
		__delay_cycles(50);
		SPI_PxOUT &= ~SCL;		// Clock Low

	}//for each bit in txChar

	LCD_PxOUT |= LCD_CS;	// Chip Select Low

}

void lcdBackspace(LCD *lcd){

	if(lcd->currentPosition == 0x00)
		lcd->currentPosition = 0x4F;
	else if(lcd->currentPosition == 0x40)
		lcd->currentPosition = 0x0F;
	else
		lcd->currentPosition--;

	lcdComByte(lcd, 0x80+lcd->currentPosition);
	lcdDataByte(lcd, ' ');
	lcdComByte(lcd, 0x80+lcd->currentPosition);

}

void redrawScreen(LCD *lcd){
	volatile unsigned int i;
	lcdComByte(lcd, 0x80);		// DDRAM position to 1st row, 1st col
	for(i=0;i<16;i++)
		lcdDataByte(lcd,lcd->memory[0][i]);
	lcdComByte(lcd,0xC0);		// DDRAm position to 2nd row, 1st col
	for(i=0;i<16;i++)
		lcdDataByte(lcd,lcd->memory[1][i]);
}
