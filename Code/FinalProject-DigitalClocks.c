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
#include "AlarmAndChimes.h"
#include "GeneralLPC.h"
#include "I2C.h"
#include "LCDModule.h"
#include "RTC.h"

#define PCONP (*(volatile unsigned int *) 0x400FC0C4)

// All on Port 0
int swColPins[] = {6, 7, 8, 9};
int swRowPins[] = {16, 15, 17, 18};

void setupKeypadPins() {
	// set all column pins to input
	for (int i = 0; i < 4; i++) {
		FIO0DIR &= ~(1<<swColPins[i]);
	}
}

/**
 * updates rowAndCol w/ pressed the row and column of the keypad
 */
void checkKeypadPress(int* rowAndCol, int loop) {
	int btnR = -1;
	int btnC = -1;

	do {
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
	} while (loop && btnR == -1 && btnC == -1);
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

// Takes row and column of buton press and decodes the value of the button
char decodeKeyPress(int r, int c) {
	if (r==0 && c==0) return '1';
	else if (r==0 && c==1) return '2';
	else if (r==0 && c==2) return '3';
	else if (r==0 && c==3) return 'A';
	else if (r==1 && c==0) return '4';
	else if (r==1 && c==1) return '5';
	else if (r==1 && c==2) return '6';
	else if (r==1 && c==3) return 'B';
	else if (r==2 && c==0) return '7';
	else if (r==2 && c==1) return '8';
	else if (r==2 && c==2) return '9';
	else if (r==2 && c==3) return 'C';
	else if (r==3 && c==0) return '*';		// not used currently
	else if (r==3 && c==1) return '0';
	else if (r==3 && c==2) return '#';		// not used currently
	else if (r==3 && c==3) return 'D';		// not used currently
	else;
	return 'x';
}

/**
 * get hours, min, secs from key presses (in order)
 * assume user is perfect and only enters valid values
 */
void getTimeFromKeyPress(int* HMS, char* time) {
	int numRC[2];

	// move cursor to second line
	LCDWriteCommand(0x80 + 0x40);
	// makes cursor visible and blinking (to indicate waiting for user input)
	LCDWriteCommand(0x0f);
	LCDWriteString(time, 8);
	// moves cursor back to start of second row (tens place of hour should be blinking)
	LCDWriteCommand(0x80 + 0x40);

	// hours digits
	checkKeypadPress(numRC, 1);
	char h10Char = decodeKeyPress(numRC[0], numRC[1]);
	LCDWriteData(h10Char);
	int h10 = 10*(h10Char - '0');

	checkKeypadPress(numRC, 1);
	char h1Char = decodeKeyPress(numRC[0], numRC[1]);
	int h1 = h1Char - '0';
	LCDWriteData(h1Char);

	LCDWriteData(':');

	// minutes digits
	checkKeypadPress(numRC, 1);
	char m10Char = decodeKeyPress(numRC[0], numRC[1]);
	LCDWriteData(m10Char);
	int m10 = 10*(m10Char- '0');

	checkKeypadPress(numRC, 1);
	char m1Char = decodeKeyPress(numRC[0], numRC[1]);
	int m1 = m1Char - '0';
	LCDWriteData(m1Char);

	LCDWriteData(':');

	// seconds digits
	checkKeypadPress(numRC, 1);
	char s10Char = decodeKeyPress(numRC[0], numRC[1]);
	int s10 = 10*(s10Char - '0');
	LCDWriteData(s10Char);
	checkKeypadPress(numRC, 1);
	char s1Char = decodeKeyPress(numRC[0], numRC[1]);
	LCDWriteData(s1Char);
	int s1 = s1Char - '0';

	HMS[0] = h10+h1;
	HMS[1] = m10+m1;
	HMS[2] = s10+s1;
}

// converts time to string, primarily used to display time on LCD
void getStringFromTime(int hour, int min, int sec, char* s) {
	s[0] = '0' + hour/10;
	s[1] = '0' + hour%10;
	s[2] = ':';

	s[3] = '0' + min/10;
	s[4] = '0' + min%10;
	s[5] = ':';

	s[6] = '0' + sec/10;
	s[7] = '0' + sec%10;
}

int main(void) {
	FIO0DIR |= (1<<10);  // TEST output LED
	setupKeypadPins();
	setup_lpc_for_I2C();

	int keyRC[2]; // row and col of pressed key

	char currentTimeStr[8];
	char alarmTimeStr[8];

	setTime(0, 0, 0);
	setAlarmTime(30, 0, 0);

	initLCD();

    while(1) {
			// checks if chime or alarm should go off
    	chimeListener();
    	alarmListener();

    	LCDWriteCommand(0x0c); // set LCD to display only (makes cursor invisible)
    	checkKeypadPress(keyRC, 0);
    	char key = decodeKeyPress(keyRC[0], keyRC[1]);

    	// set time clock (C)
    	while (key == 'C') {
    		int HMS[3];
			LCDWriteCommand(0x01); wait_us(4000);
			LCDWriteString("SET TIME: ", 10);
    		getTimeFromKeyPress(HMS, currentTimeStr);
    		setTime(HMS[2], HMS[1], HMS[0]);
    		key = 'x';
    	}

    	// set alarm clock (A)
    	while (key == 'A') {
    		int HMS[3];
			LCDWriteCommand(0x01); wait_us(4000);
			LCDWriteString("SET ALARM: ", 11);
			getTimeFromKeyPress(HMS, alarmTimeStr);
			setAlarmTime(HMS[2], HMS[1], HMS[0]);
			key = 'x';
    	}

		// get times from clock and alarm
		getStringFromTime(HOUR, MIN, SEC, currentTimeStr);
		getStringFromTime(ALHOUR, ALMIN, ALSEC, alarmTimeStr);

		// clear display and move cursor to top left
		LCDWriteCommand(0x01); wait_us(4000);

		// display clock time on LCD
		LCDWriteString("TIME : ", 7);
		LCDWriteString(currentTimeStr, 8);

		// move cursor to second line
		LCDWriteCommand(0x80 + 0x40);

		// display alarm time on LCD
		LCDWriteString("ALARM: ", 7);
		LCDWriteString(alarmTimeStr, 8);

		// allow LCD to process the info
		wait(0.10);
    }
    return 0;
}
