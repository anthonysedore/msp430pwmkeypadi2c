#include <msp430.h>
#include "LiquidCrystal_I2C.h"
#include <string.h>

unsigned int TXBUF;				// The value to put in the buffer to transmit information.

void I2C_Init(int addr){
    UCB0CTLW0 |= UCSWRST;       // Put eUSCI_B0 into software reset

    //-- Configure uUSCI_B0-------------------------------------------
    UCB0CTLW0 |= UCSSEL_3;      // Choose BRCLK = SMCLK = 1MHz
    UCB0BRW = 10;               // Divide BRCLK by 10 for SCL = 100kHz

    UCB0CTLW0 |= UCMODE_3;      // Put into I2C Mode
    UCB0CTLW0 |= UCMST;         // Put into Master Mode
    UCB0CTLW0 |= UCTR;          // Put into Tx Mode
    UCB0I2CSA = addr;           // Slave Address = 0x27

    UCB0CTLW1 |= UCASTP_2;      // Auto stop when UCB0TBCNT reached
    UCB0TBCNT =0x01;            // Send 1 byte of data

    //-- Configure Ports----------------------------------------------
    P1SEL1 &= ~BIT3;            // P1.3 = SCL
    P1SEL0 |=  BIT3;

    P1SEL1 &= ~BIT2;            // P1.2 = SDA
    P1SEL0 |=  BIT2;

    PM5CTL0 &= ~LOCKLPM5;       // Turn on GPIO
    //-- Take eUSCI_B0 out of SW reset---------------------------------
    UCB0CTLW0 &= ~UCSWRST;      // Put eUSCI_B0 out of software reset

    UCB0IE |= UCTXIE0;          // Enable I2C_B0 Tx IRQ
    __enable_interrupt();
}
void I2C_Send (int value){
    UCB0CTLW0 |= UCTXSTT;		// Generate a START condition.
    TXBUF = value;				// Put what you want (store the data) to transmit into the Tx buffer register. See the interrupt vector below.
}
void pulseEnable (int value){
    I2C_Send (value | En);  	// En high
    __delay_cycles(150);      	// enable pulse must be >450ns
    I2C_Send(value  & ~En); 	// En low
    __delay_cycles(1500);       // commands need > 37us to settle
}
void write4bits (int value) {
    I2C_Send (value);
    __delay_cycles(50);
    pulseEnable (value);
}
void LCD_Send(int value, int mode) {
    int high_b = value & 0xF0;
    int low_b = (value << 4) & 0xF0;			// Shift the bits to the left and then set all bits except the ones in 0xF0 to 0.
    write4bits ( high_b | mode);				// write4bits is a function call with one arg and the arg is the result of a bitwise or | (one pipe symbol).
    write4bits ( low_b  | mode);				// The arg of write4bits uses 4 bits for the value and 4 bits for the mode.
    											// It is being called first with high bits, then with th elow bits to write 8 bits.
}
void LCD_Write (char *text){
    unsigned int i;
    for (i=0; i < strlen(text); i++){
        LCD_Send((int)text[i], Rs | LCD_BACKLIGHT);
    }
}
void LCD_WriteNum(unsigned int num) {
    unsigned int reverseNum = 0;       
   	unsigned int digits = 0;					// To use as a digit counter. For now, no digits are counted yet until we enter the first while loop.
	int i;										// This is for the for loop to run digits iterations.
	
	if (num == 0) {								// If the user input 0 on the keypad...
		LCD_Send(0 | 0x30, Rs | LCD_BACKLIGHT);	// ...then display 0 for 0% duty cycle.
	}
	else {
								
	    while (num > 0) {
	        reverseNum = reverseNum * 10 + (num % 10);
	        num /= 10;
	        digits++;							// Increment digits; this means it is counting how many digits the user input from the keypad.
	    }
	    
	    for(i = 0; i < digits; ++i) {			/* It will run digits iterations while it does the modulo and division operation. Now it knows how many digits
	    									 	it will print. This fixes the zeroes issue; now it can display #0 and 100 on the LCD successfully. */
	    	LCD_Send((reverseNum % 10) | 0x30, Rs | LCD_BACKLIGHT);
	    	reverseNum /= 10;
		}
	}
}
void LCD_SetCursor(int col, int row) {		// This function converts a column and row to a single number the LCD is expecting to set the cursor position.
    int row_offsets[] = { 0x00, 0x40, 0x14, 0x54 };	// The LCD must have an interface where the starting position of each row is 0x00, 0x40, 0x14, and 0x54.
    LCD_Send(LCD_SETDDRAMADDR | (col + row_offsets[row]),  LCD_BACKLIGHT);
}
void LCD_ClearDisplay(void){
    LCD_Send(LCD_CLEARDISPLAY,  LCD_BACKLIGHT);
    __delay_cycles(50);
}
void LCD_leftToRight(void) {
    LCD_Send(LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT,  LCD_BACKLIGHT);
}
void LCD_rightToLeft(void) {
    LCD_Send(LCD_ENTRYMODESET | LCD_ENTRYRIGHT | LCD_ENTRYSHIFTDECREMENT,  LCD_BACKLIGHT);
}
void LCD_Setup(void){
    int _init[] = {LCD_init, LCD_init, LCD_init, LCD_4_BIT};
    int _setup[5];
    int mode = LCD_BACKLIGHT;
    _setup[0] = LCD_FUNCTIONSET | LCD_4BITMODE | LCD_2LINE | LCD_5x8DOTS;
    _setup[1] = LCD_CLEARDISPLAY;
    _setup[2] = LCD_RETURNHOME;
    _setup[3] = LCD_ENTRYMODESET | LCD_ENTRYLEFT | LCD_ENTRYSHIFTDECREMENT;
    _setup[4] = LCD_DISPLAYCONTROL | LCD_DISPLAYON | LCD_CURSOROFF | LCD_BLINKOFF;

    write4bits(_init[0]);	// Waiting for the enable function to be written.
    __delay_cycles(108000);//__delay_cycles(4500*us);  <--- equivalent to this.		 It is the value we need to establish between the enable.
    write4bits(_init[1]);
    __delay_cycles(108000);
    write4bits(_init[2]);
    __delay_cycles(3600);
    write4bits(_init[3]);

    LCD_Send(_setup[0], mode);
    LCD_Send(_setup[1], mode);
    __delay_cycles(50);
    LCD_Send(_setup[2], mode);
    LCD_Send(_setup[3], mode);
    LCD_Send(_setup[4], mode);
}
#pragma vector = EUSCI_B0_VECTOR
__interrupt void EUSCI_B0_I2C_ISR(void){
    UCB0TXBUF = TXBUF;						// The value that we want to transmit is in the buffer.
}