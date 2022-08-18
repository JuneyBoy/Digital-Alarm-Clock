/*
 * AlarmAndChimes.h
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#ifndef AlarmAndChimes_H_
#define AlarmAndChimes_H_

// PWM Registers
#define PWM1MCR (*(volatile unsigned int *) 0x40018014)
#define PWM1MR0 (*(volatile unsigned int *) 0x40018018)
#define PWM1MR1 (*(volatile unsigned int *) 0x4001801C)
#define PWM1PCR (*(volatile unsigned int *) 0x4001804C)
#define PWM1TCR (*(volatile unsigned int *) 0x40018004)
#define PWM1TC (*(volatile unsigned int *) 0x40018008)
#define PWM1LER (*(volatile unsigned int *) 0x40018050)

/**
 * Initialize PWM to FREQ = PCLK / periodParam
 */
void setupPWM(int period, float dc);

void setAlarmTime(int sec, int min, int hour);

void alarmListener();

void chimeListener();

void playChimes(int* chime);

void playAlarm();

#endif /* AlarmAndChimes_H_ */
