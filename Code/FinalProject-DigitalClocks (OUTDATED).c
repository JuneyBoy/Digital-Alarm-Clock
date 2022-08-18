/*
===============================================================================
 Name        : FinalProject-DigitalClocks.c
 Author      : $(Arjun Ganesan, Daniel Ngo)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC17xx.h"
#endif

#include <cr_section_macros.h>

#define PCONP (*(volatile unsigned int *) 0x400FC0C4)
#define PCLKSEL0 (*(volatile unsigned int *) 0x400FC1A8)
#define PINSEL1 (*(volatile unsigned int *) 0x4002C004)
#define PINSEL4 (*(volatile unsigned int *) 0x4002C010)
#define RSLCDPin 4
#define ELCDPin 5

// LPC1769 definitions
#define I2C0CONSET (*(volatile unsigned int *)0x4001C000)
#define I2C0STAT (*(volatile unsigned int *)0x4001C004)
#define I2C0DAT (*(volatile unsigned int *)0x4001C008)
#define I2C0SCLH (*(volatile unsigned int *)0x4001C010)
#define I2C0SCLL (*(volatile unsigned int *)0x4001C014)
#define I2C0CONCLR (*(volatile unsigned int *)0x4001C018)

// MCP23017 I/O expander definitions
#define MCP_OPCODE (volatile unsigned int)0b0100000
#define MCP_GPIOA (volatile unsigned int)0x12
#define MCP_GPIOB (volatile unsigned int)0x13
#define MCP_IODIRA (volatile unsigned int)0x00
#define MCP_IODIRB (volatile unsigned int)0x01

// READ/WRITE bit constants
#define WRITE (volatile unsigned int)0
#define READ (volatile unsigned int)1

// Digital-to-Analog registers
#define DACR (*(volatile unsigned int *) 0x4008C000)

// PWM Registers
#define PWM1MCR (*(volatile unsigned int *) 0x40018014)
#define PWM1MR0 (*(volatile unsigned int *) 0x40018018)
#define PWM1MR1 (*(volatile unsigned int *) 0x4001801C)
#define PWM1PCR (*(volatile unsigned int *) 0x4001804C)
#define PWM1TCR (*(volatile unsigned int *) 0x40018004)
#define PWM1TC (*(volatile unsigned int *) 0x40018008)
#define PWM1LER (*(volatile unsigned int *) 0x40018050)

// Interrupts
#define ISER0 (*(volatile unsigned int *) 0xE000E100)
#define ISER1 (*(volatile unsigned int *) 0xE000E104)
#define IO0IntClr (*(volatile unsigned int *) 0x4002808C)
#define IO0IntEnR (*(volatile unsigned int *) 0x40028090)
#define IO0IntEnF (*(volatile unsigned int *) 0x40028094)
#define IO0IntStatR (*(volatile unsigned int *) 0x40028084)
#define IO0IntStatF (*(volatile unsigned int *) 0x40028088)

#define FIO0DIR (*(volatile unsigned int *) 0x2009C000)
#define FIO0PIN (*(volatile unsigned int *) 0x2009C014)

#define T0TCR (*(volatile unsigned int *)0x40004004)
#define T0TC (*(volatile unsigned int *)0x40004008)

// RTC Registers (P572)
#define CCR (*(volatile unsigned int *)0x40024008)

#define SEC (*(volatile unsigned int *)0x40024020)
#define MIN (*(volatile unsigned int *)0x40024024)
#define HOUR (*(volatile unsigned int *)0x40024028)

#define ALSEC (*(volatile unsigned int *)0x40024060)
#define ALMIN (*(volatile unsigned int *)0x40024064)
#define ALHOUR (*(volatile unsigned int *)0x40024068)

#define CTIME0 (*(volatile unsigned int *)0x40024014)

// C4, G4, E4, A4, B4, A4, G4, E4, C4
int melody_notes[] = { 261, 392, 329, 440, 494, 440, 392, 329, 261 };

// time length of quarter note
float qNoteTime = 1.0;

// G#4, F#4, E4, B3
int wmChime1[] = { 415, 370, 330, 247 };
// E4, G#4, F#4, B3
int wmChime2[] = { 330, 415, 370, 247 };
// E4, F#4, G#4, E4
int wmChime3[] = { 330, 370, 415, 330 };
// G#4, E4, F#4, B3
int wmChime4[] = { 415, 330, 370, 247 };
// B3, F#4, G#4, E4
int wmChime5[] = { 247, 370, 415, 330 };

// All on Port 0
int swColPins[] = {6, 7, 8, 9};
int swRowPins[] = {16, 15, 17, 18};

//functions taken from Timer Lecture
void wait_us(int us){
	int start = T0TC;  // note starting time
	T0TCR |= (1<<0);   // start timer
	while ((T0TC-start)<us) {} // wait for time to pass
}
void wait(float sec){
	wait_us(sec*1000000); // convert seconds to microseconds
}

void setupPins() {
	// set all column pins to input
	for (int i = 0; i < 4; i++) {
		FIO0DIR &= ~(1<<swColPins[i]);
	}
}

/**
 * Initialize PWM to FREQ = PCLK / periodParam
 */
void setupPWM(int period, float dc) {
	PCLKSEL0 |= (1<<12); 					// PWM PCLK = CCLK/1
	PWM1TCR = 0; PWM1TC = 0;
	PWM1MR0 = period; 						// set PCLK cycle period (8-bit equivalent)
	PINSEL4 |= ~(1<<1); PINSEL4 |= (1<<0);  // Configure P2.0 as PWM1.1
	PWM1MCR = (1<<1); 						// Reset counter on MR0 match
	PWM1PCR = (1<<9); 						// Single edge mode and enable PWM1.1 only
	PWM1TCR = (1<<0) | (1<<3); 				// Enable counter and PWM modeW
	PWM1MR1 = (int)(period * dc); 			// change MR1 to output new 8-bit D sample value (50% DC)
	PWM1LER = (0b11<<0); 					// set bits 1:0 to 11 to use new MR1 and MR0 values
}

// Look for rising edges on P0.5 and falling edges on P0.5
void GPIOinterruptInitialize(void)
{
  IO0IntClr = (1<<5); // clear any old events on P0.5
  IO0IntEnR |= (1<<5); // enable rising edge on P0.5
  IO0IntEnF |= (1<<5); // enable falling edge on P0.5
  ISER0 = (1<<21); // enable EINT3 (GPIO) interrupts
}

void EINT3_IRQHandler(void)
{
  if ((IO0IntStatR>>5) & 1) { // rising edge on P0.5
	  setupPWM(7648, 0.5); 	// second door bell: 523 Hz
	  IO0IntClr = (1<<5); // clear this event on P0.5
  }

  if ((IO0IntStatF>>5) & 1) { // falling edge on P0.5
	  setupPWM(6069, 0.5); 	// first door bell: 659 Hz
	  IO0IntClr = (1<<5); // clear this event on P0.5
  }
}

/**
 * updates rowAndCol w/ pressed the row and column of the keypad
 */
void checkKeypadPress(int* rowAndCol) {
	int btnR = -1;
	int btnC = -1;

	// constantly go thru each row of keypad
	for (int r = 0; r < 4; r++) {
		// select current row (set to output)
		FIO0DIR |= (1<<swRowPins[r]);	// output
		FIO0PIN &= ~(1<<swRowPins[r]);	// drive low

		// check which key is pressed
		for (int c = 0; c < 4; c++) {
			if(~(FIO0PIN>>swColPins[c]) & 1) {
				// wait until user lets go
				while (~(FIO0PIN>>swColPins[c]) & 1);

				// update pressed key row and column
				btnR = r;
				btnC = c;
				FIO0PIN |= (1<<10); // turn on test LED
				break;
			} else {
				btnR = -1;
				btnC= -1;
				FIO0PIN &= ~(1<<10); // turn off test LED
			}
		}

		wait_us(1);

		// update input array
		rowAndCol[0] = btnR;
		rowAndCol[1] = btnC;

		// de-select current row (set to input) for next loop
		FIO0DIR &= ~(1<<swRowPins[r]);
		wait_us(1);

		// key has been pressed already
		if (btnR != -1 && btnC != -1) break;
	}
}

void setTime(int sec, int min, int hour) {
	CCR &= ~(1<<0); // disable clock
	CCR |=(1<<1); // reset clock

	// assign time
	SEC = sec;
	MIN = min;
	HOUR = hour;

	CCR = (1<<0); // enable clock
}

void setAlarmTime(int sec, int min, int hour) {
	ALSEC |= sec;
	ALMIN |= min;
	ALHOUR |= hour;
}

void alarmListener() {
	int alarm_flg = 0;
	if (!alarm_flg &&
			(HOUR == ALHOUR && MIN == ALMIN && SEC >= ALSEC) ||
			(HOUR == ALHOUR+1 && MIN == 0 && SEC <= ALSEC)
	) {
		alarm_flg = 1;
		playAlarm();
	}
	alarm_flg = 0;
}
void chimeListener() {
	if (MIN == 00 && SEC <= 5) {
		playChimes(wmChime2);
		playChimes(wmChime3);
		playChimes(wmChime4);
		playChimes(wmChime5);
		wait(1);
		// chimes whole notes (E3) for each hour (1 strike for 1 o'clock, 2 strikes for 2 o'clock, etc.)
		for(int i = 0; i < HOUR % 12; i++) {
			setupPWM((int)(4000000/162), 0.5);
			wait(4*qNoteTime);
			setupPWM(1000, 0);
		}
	} else if (MIN == 15 && SEC <= 5) {
			playChimes(wmChime1);
	} else if (MIN == 30 && SEC <= 5) {
			playChimes(wmChime2);
			playChimes(wmChime3);
	} else if (MIN == 45 && SEC <= 5) {
			playChimes(wmChime4);
			playChimes(wmChime5);
			playChimes(wmChime1);
	} else; // do nothing
}

void playChimes(int* chime) {
	for(int i = 0; i < 4; i++) {
		setupPWM((int)(4000000/chime[i]), 0.5);
		if(i != 3) {
			wait(qNoteTime);
		}
		else {
			wait(2*qNoteTime);
		}
	}
	setupPWM(1000, 0.0);
}

void playAlarm() {
	for(int i = 0; i < 8; i++) {
		setupPWM((int)(4000000/melody_notes[i]), 0.5);
		wait(qNoteTime);
	}
	setupPWM(1000, 0.0);
}

void initLCD() {
	// configure RS, R/W, and E as outputs to LCD and drive low
	FIO0PIN &= ~(1<<RSLCDPin);
	FIO0PIN &= ~(1<<ELCDPin);

	// wait 4 ms
	wait_us(4000);
	// write 0x38
	writeToGPIOA(0x38);
	// write 0x06
	writeToGPIOA(0x06);
	// write 0x0c
	writeToGPIOA(0x0c);
	// write 0x01
	writeToGPIOA(0x01);
}

void LCDWriteCommand(int commandCode) {
	// update D0-D7 with command code
	writeToGPIOA(commandCode);
	// drive RS low
	FIO0PIN &= ~(1<<RSLCDPin);
	// drive E high then low (might need short delay)
	FIO0PIN |= (1<<ELCDPin);
	wait_us(10);
	FIO0PIN &= ~(1<<ELCDPin);
	// wait 100 us
	wait_us(100);
}

void LCDWriteData(int data) {
	// update D0-D7 with data
	writeToGPIOA(data);
	// drive RS high
	FIO0PIN |= (1<<RSLCDPin);
	// drive E high then low (might need short delay)
	FIO0PIN |= (1<<ELCDPin);
	wait_us(10);
	FIO0PIN &= ~(1<<ELCDPin);
	// wait 100 us
	wait_us(100);
}

void start() {
	I2C0CONSET = (1<<3); // set SI bit
	I2C0CONSET = (1<<5); // set STA bit
	I2C0CONCLR = (1<<3); // clear SI bit
	while (((I2C0CONSET>>3) & 1) != 1); // wait for SI bit to return to 1
	I2C0CONCLR = (1<<5); // clear STA bit
}
void stop() {
	I2C0CONSET = (1<<4); // set STO bit
	I2C0CONCLR = (1<<3); // clear SI bit
	while (((I2C0CONSET>>4) & 1) != 0); // wait for STO bit to return to 0
}

/**
 * @param 8-bit input data to write
 * write data to I2C given 8-bit data
 */
void write(int data){
	I2C0DAT = data; // assign data to I2CxDAT
	I2C0CONCLR = (1<<3); // clear SI bit
	while (((I2C0CONSET>>3) & 1) != 1); // wait for SI bit to return to 1
	// optional: check I2CxSTAT for acknowledge
}

/**
 * @param 8-bit input data to write
 * writes data to GPIOA
 */
void writeToGPIOA(int data){
	start();
	write((MCP_OPCODE<<1) | (WRITE<<0));
	write(MCP_GPIOA);
	write(data);
	stop();
}

/**
 * Sets up LPC1769 for I2C operation
 */
void setup_lpc() {
	// set up PINSEL for I2C SDA0
	PINSEL1 &= ~(1<<23); PINSEL1 |= (1<<22);

	// set up PINSEL for I2C SCL0
	PINSEL1 &= ~(1<<25); PINSEL1 |= (1<<24);

	// FREQUENCY = PCLK / (I2CxSCLH + I2CxSCLL)
	// PCLK = CCLK/4 by default but can change with PCLKSELx
	// CCLK default is 4 MHz

	// set up PCLKSEL0 bit 15:14 to 00 (PCLK_I2C0)
	PCLKSEL0 &= ~(1<<15) & ~(1<<14);

	// f=100kHz, 50% duty cycle
	I2C0SCLL = 5;
	I2C0SCLH = I2C0SCLL + 1;

	// clr and reset en bit
	I2C0CONCLR = (1<<6);
	I2C0CONSET = (1<<6);
}

/**
 * Sets up IO Expander for I2C operation
 */
void setup_mcp() {
	// configure GPIOA and GPIOB as output
	start();
	write((MCP_OPCODE<<1) | (WRITE<<0));
	write(MCP_IODIRA);
	write(0x00);
	stop();

	start();
	write((MCP_OPCODE<<1) | (WRITE<<0));
	write(MCP_IODIRB);
	write(0x80);
	stop();
}

int main(void) {
	FIO0DIR |= (1<<10);  // TEST output LED
	setupPins();
	setup_lpc();
	setup_mcp();

	int keyRC[2]; // row and col of pressed key

	setTime(0, 0, 0);
	setAlarmTime(0, 0, 30);
	initLCD();

    while(1) {
    	chimeListener();
    	alarmListener();
    	checkKeypadPress(keyRC);

    	// set time clock (C)
    	while (keyRC[0] == 3 && keyRC[1] == 2) {

    	}

    	// set alarm clock (A)
    	while (keyRC[0] == 3 && keyRC[1] == 0) {

    	}

    	int currentSec = SEC;
    	int currentMin = MIN;
    	int currentHour = HOUR;
    }
    return 0;
}
