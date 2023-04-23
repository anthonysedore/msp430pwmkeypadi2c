#include <msp430.h>
#include "LiquidCrystal_I2C.h"
#include <stdio.h>
#include <string.h>

// We are using P3 for the keypad. The columns are inputs and rows are outputs.
#define keyport P3OUT

// Column pins are P3.0, P3.1, P3.2, and P3.3.
#define COL1 (P3IN & 0x10)      // Input pins for the columns.
#define COL2 (P3IN & 0x20)
#define COL3 (P3IN & 0x40)
#define COL4 (P3IN & 0x80)

unsigned i, k, key = 0;

unsigned char Key_Val[] = {' ','1','2','3','A','4','5','6','B','7','8','9','C','*','0','#','D'}; // ' ' is empty, index 0 is dummy; does not return anything.

// User-defined functions prototypes:
unsigned char get_key(void);
void DelayMs(unsigned int Ms);

unsigned int DutyCycle = 0;          //Duty Cycle initially set to 0

void main(void) {
    WDTCTL = WDTPW | WDTHOLD;   // Stop WDT

    P3DIR = 0x0F;               // Output registers for the ROWS. Pins: P3.4, P3.5, P3.6, P3.7.

    // Pull-up resistor initialization:
    P3REN = 0xFF;               // These pins will be associated with a resistor type. Should correlate with input pins (columns).
    P3OUT = 0xF0;               // Set P3 with pull-up resistor type

    P4DIR |= 0xC1; //Set P4.0, 4.6, 4.7 as output,
    P4OUT &= ~0xC1; // clear P4 pins
    P4OUT |= 0x40; //Turn P4.6 ON, to make motor spin IN1 HIGH and IN2 LOW

    PM5CTL0 &= ~LOCKLPM5;       // Disable high-impedance mode.

    TB0CTL = TBSSEL__ACLK + MC__UP + TBCLR;      // ACLK, upmode, clear
    TB0CCR0 = 1000;                              // PWM Period, Timer resets when = TB0CCR0, PWM is 32.75 Hz, optimal is 50-100 Hz range for h bridge, shrug
    TB0CCR1 = 0;                                 // Duty Length, TB0CCR1/TB0CCR0 = duty cycle
    TB0CCTL1 = CCIE;                             // CCR0 interrupt enabled
    TB0CCTL1&=~(CCIFG);                          // Clear interrupt flag
    TB0CCTL0 = CCIE;                             // CCR1 interrupt enabled
    TB0CCTL0&=~(CCIFG);                          // Clear interrupt flag

    __enable_interrupt();                        //Enable maskable interrupts

    // LCD function call and setups:
    I2C_Init(0x27);             // 0x27 signifies the slave address (address of the LCD with the I/O expander).
    LCD_Setup();
    LCD_SetCursor(0, 0);        // Initial position for the cursor at row 1, column 1.
    TB0CCR1 = 0;    //Set Duty Length to 0 for initial to keep motor off

    while (1) {
        //Clear LCD and Display Duty Cycle and ask for input
        LCD_ClearDisplay();
        LCD_SetCursor(0, 0);
        LCD_Write("DutyCycle=");
        LCD_WriteNum(DutyCycle);
        LCD_SetCursor(0, 1);
        LCD_Write("Input: ");

        //Default Values for polling keys
        unsigned char number = '0';
        unsigned char numbers[8] = {0};
        DutyCycle = 0;
        unsigned int counter = 0;

        while (number != '*') { //Keeps Polling Keypad until * is pressed
            number = get_key();
            if (number >= '0' && number <= '9') { //Only Accepts Number Buttons
                numbers[counter++] = number; //Store key into array of key_presses
                LCD_WriteNum((int)number-'0');
            }
        }
        sscanf(numbers, "%d", &DutyCycle); //Converts char array of key presses into integer
        if (DutyCycle >= 100) {  //If Input Value was bigger than 100, set duty cycle to 100
            DutyCycle = 100;
            TB0CCR1 = 1001; //Just to set TB0CCR1 more than TB0CCR0, if they equal there becomes unpredictable errors with interrupts
        }
        else {
            TB0CCR1 = DutyCycle*10; //Duty Cycle, ex 50 convert to 500 so TB0CCR1/TB0CCR0 (TB0CCR0 = 1000) = 0.50 <-desired duty cycle
        }
    }
}

unsigned char get_key(void) {            // A number associated with the key they pressed.

    k=1;                                 // k is just a number to increment by 4, so there will be a different return value for every key.
                                         // We start initializing k as 1.

    for(i = 0; i < 4; i++) {             // To assign different key presses. Send 0 to bit i.
        keyport = ((0x01 << i) ^ 0xff);  // Shift left is setting the nth bit to 1, then it inverts the bit to set it to 0 and other bits to 1.
                                         // Every loop iteration is sending one zero and sending it it gets back a zero on one of the pins.
                                         // There are four locations it needs to send a zero, and for each of those locations there are four
                                         // places it needs to check if it got a zero back.
                                         // In summary: sending 0 to bit 1, 2, 3, 4 and see if getting 0 from different columns.

        if(!COL1) {
            key = k+0;
            while(!COL1);
            DelayMs(50);                 // Add a small delay of 0.05s. Every time we check a row, the button debouncing concept is utilized.
            return Key_Val[key];
        }

        if(!COL2) {
            key = k+1;
            while(!COL2);
            DelayMs(50);
            return Key_Val[key];
        }

        if(!COL3) {
            key = k+2;
            while(!COL3);
            DelayMs(50);
            return Key_Val[key];
        }

        if(!COL4) {
            key = k+3;
            while(!COL4);
            DelayMs(50);
            return Key_Val[key];
        }

    k+=4;                     // This is k += 4 because we checked four values; we want a different return value for each possible key press.
    keyport |= (0x01 << i);   // It is setting the bit it set back to 1. This is to stop sending a 0, in other words to stop checking that row.

    } // End of for loop.
  return 0;     // Return the unsigned char to main.
 }  // End of UDF get_key.

void DelayMs(unsigned int Ms) {
    while(Ms) {
        __delay_cycles(1000);
        Ms--;
    }
}   // End of DelayMs user-defined function.

//When timer = TB0CCR0
#pragma vector=TIMER0_B0_VECTOR
__interrupt void TMR0 ()
{
    P4OUT |= 1; //Turns On P4.0
    TB0CCTL0 &= ~(CCIFG);           // Clear interrupt flag
}

//When timer = TB0CCR1
#pragma vector=TIMER0_B1_VECTOR
__interrupt void TMR1 ()
{
    P4OUT &= ~1; //Turn off P4.0
    TB0CCTL1 &= ~(CCIFG);           // Clear interrupt flag
}
