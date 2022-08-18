/*
 * AlarmAndChimes.c
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#include "GeneralLPC.h"
#include "AlarmAndChimes.h"
#include "RTC.h"

// C4, G4, E4, A4, B4, A4, G4, E4, C4
int melody_notes[] = { 261, 392, 329, 440, 494, 440, 392, 329, 261 };

// time length of quarter note
float qNoteTime = 0.50;

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

void setAlarmTime(int sec, int min, int hour) {
	ALSEC = sec;
	ALMIN = min;
	ALHOUR = hour;
}

int alarm_flg = 0;  // marks if alarm has played
int chime_flg = 0;  // marks if chime has played

// checks to see if alarm should go off
void alarmListener() {
	// if alarm has not already played and the alarm time is close to the current time, set alarm flag to 1 and play alarm
	// gives some buffer time by only checking if the current time is some seconds greater than the alarm time rather than checking if the current second matches the exact alarm second
	if (!alarm_flg &&
		((HOUR == ALHOUR && MIN == ALMIN && SEC >= ALSEC))
	) {
		alarm_flg = 1;
		LCDWriteCommand(0x01); wait_us(4000);
		LCDWriteString("ALARM!", 6);
		playAlarm();
	} else if (MIN != ALMIN) alarm_flg = 0;	// if the current minute does not match the alarm minute, then the alarm flag is reset to 0
}

// checks to see if chimes should go off
void chimeListener() {
	if (!chime_flg) {
		if (MIN == 00) {
			chime_flg = 1;
			// clear display and move cursor to top left
			LCDWriteCommand(0x01); wait_us(4000);
			LCDWriteString("TOP OF THE HOUR!", 16);
			playChimes(wmChime2);
			playChimes(wmChime3);
			playChimes(wmChime4);
			playChimes(wmChime5);
			wait(1);
			// chimes whole notes (E3) for each hour (1 strike for 1 o'clock, 2 strikes for 2 o'clock, etc.)
			for(int i = 0; i < HOUR % 12; i++) {
				setupPWM((int)(4000000/165), 0.5);
				wait(4*qNoteTime);
				setupPWM(1000, 0);
				wait(0.25*qNoteTime);
			}
		} else if (MIN == 15) {
			chime_flg = 1;
			LCDWriteCommand(0x01); wait_us(4000);
			LCDWriteString("15 PAST THE HOUR!", 17);
			playChimes(wmChime1);
		} else if (MIN == 30) {
			chime_flg = 1;
			LCDWriteCommand(0x01); wait_us(4000);
			LCDWriteString("HALF PAST THE HOUR!", 19);
			playChimes(wmChime2);
			playChimes(wmChime3);
		} else if (MIN == 45 && SEC <= 3) {
			chime_flg = 1;
			LCDWriteCommand(0x01); wait_us(4000);
			LCDWriteString("15 TIL THE HOUR!", 19);
			playChimes(wmChime4);
			playChimes(wmChime5);
			playChimes(wmChime1);
		} else; // do nothing
	} else if (MIN % 15 != 0) chime_flg = 0;
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
