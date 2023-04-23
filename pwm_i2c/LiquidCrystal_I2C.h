/*
 * LiquidCrystal_I2C.h
 */

#ifndef LIQUIDCRYSTAL_I2C_H_
#define LIQUIDCRYSTAL_I2C_H_

extern unsigned int TXBUF;

#define LCD_4_BIT 0x20
#define LCD_init  0x30

// commands
#define LCD_CLEARDISPLAY 0x01		// These are shortcuts for the commands in hex.
#define LCD_RETURNHOME 0x02
#define LCD_ENTRYMODESET 0x04
#define LCD_DISPLAYCONTROL 0x08
#define LCD_CURSORSHIFT 0x10
#define LCD_FUNCTIONSET 0x20
#define LCD_SETCGRAMADDR 0x40
#define LCD_SETDDRAMADDR 0x80

// flags for display entry mode
#define LCD_ENTRYRIGHT 0x00
#define LCD_ENTRYLEFT 0x02
#define LCD_ENTRYSHIFTINCREMENT 0x01
#define LCD_ENTRYSHIFTDECREMENT 0x00

// flags for display on/off control
#define LCD_DISPLAYON 0x04
#define LCD_DISPLAYOFF 0x00
#define LCD_CURSORON 0x02
#define LCD_CURSOROFF 0x00
#define LCD_BLINKON 0x01
#define LCD_BLINKOFF 0x00

// flags for display/cursor shift
#define LCD_DISPLAYMOVE 0x08
#define LCD_CURSORMOVE 0x00
#define LCD_MOVERIGHT 0x04
#define LCD_MOVELEFT 0x00

// flags for function set
#define LCD_8BITMODE 0x10		// These are standard for 2x16
#define LCD_4BITMODE 0x00
#define LCD_2LINE 0x08
#define LCD_1LINE 0x00
#define LCD_5x10DOTS 0x04
#define LCD_5x8DOTS 0x00

// flags for backlight control
#define LCD_BACKLIGHT 0x08
#define LCD_NOBACKLIGHT 0x00

#define En 0x04  // Enable bit
#define Rw 0x02  // Read/Write bit
#define Rs 0x01  // Register select bit

// Declare the functions in the header:
void I2C_Init(int addr);		// Address of the device in the argument.
void I2C_Send (int value);
void pulseEnable (int value);
void write4bits (int value);
void LCD_Send(int value, int mode) ;
void LCD_Write (char *text);
void LCD_WriteNum(unsigned int num);
void LCD_SetCursor(int col, int row);
void LCD_ClearDisplay(void);
void LCD_leftToRight(void) ;
void LCD_rightToLeft(void) ;
void LCD_Setup(void);

#endif /* LIQUIDCRYSTAL_I2C_H_ */
