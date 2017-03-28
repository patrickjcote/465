#ifndef LCD_H_
#define LCD_H_

#define LCD_RS		BIT7
#define LCD_CS		BIT6
#define LCD_PxOUT 	P1OUT
#define LCD_PxDIR 	P1DIR

#define MOSI		BIT1
#define SCL			BIT0
#define SPI_PxDIR	P1DIR
#define SPI_PxIN	P1IN
#define SPI_PxOUT	P1OUT
#define SPI_PERIOD	50

#define _RS_Command() 	LCD_PxOUT &= ~LCD_RS
#define _RS_Data()		LCD_PxOUT |= LCD_RS

typedef struct lcds {
	volatile unsigned int currentPosition;
	volatile unsigned char memory[2][16];
} LCD;


void lcdInit(LCD *);
void lcdComByte(LCD *,char);
void lcdDataByte(LCD *,char);
void lcdBackspace(LCD *);



#endif
