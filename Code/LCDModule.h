/*
 * LCDModule.h
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#ifndef LCDModule_H_
#define LCDModule_H_

#define RSLCDPin (volatile unsigned int) 4
#define ELCDPin (volatile unsigned int) 5

void initLCD();

void LCDWriteCommand(int commandCode);

void LCDWriteData(int data);

void LCDWriteString(char string[], int length);

#endif /* LCDModule_H_ */
