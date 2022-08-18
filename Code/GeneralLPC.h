/*
 * GeneralLPC.h
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#ifndef GeneralLPC_H_
#define GeneralLPC_H_

#define PCLKSEL0 (*(volatile unsigned int *) 0x400FC1A8)
#define PINSEL1 (*(volatile unsigned int *) 0x4002C004)
#define PINSEL4 (*(volatile unsigned int *) 0x4002C010)

#define FIO0DIR (*(volatile unsigned int *) 0x2009C000)
#define FIO0PIN (*(volatile unsigned int *) 0x2009C014)

#define T0TCR (*(volatile unsigned int *) 0x40004004)
#define T0TC (*(volatile unsigned int *) 0x40004008)

//functions taken from Timer Lecture
void wait_us(int us);
void wait(float sec);

#endif /* GeneralLPC_H_ */
