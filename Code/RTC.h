/*
 * RTC.h
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#ifndef RTC_H_
#define RTC_H_

// RTC Registers (P572)
#define CCR (*(volatile unsigned int *)0x40024008)

#define SEC (*(volatile unsigned int *)0x40024020)
#define MIN (*(volatile unsigned int *)0x40024024)
#define HOUR (*(volatile unsigned int *)0x40024028)

#define ALSEC (*(volatile unsigned int *)0x40024060)
#define ALMIN (*(volatile unsigned int *)0x40024064)
#define ALHOUR (*(volatile unsigned int *)0x40024068)

#define CTIME0 (*(volatile unsigned int *)0x40024014)

#endif /* RTC_H_ */
