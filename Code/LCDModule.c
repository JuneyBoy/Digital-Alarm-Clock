/*
 * LCDModule.c
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#include "GeneralLPC.h"
#include "I2C.h"
#include "LCDModule.h"

void initLCD() {
	// configure RS, R/W, and E as outputs to LCD and drive low
	FIO0DIR |= (1<<RSLCDPin);
	FIO0DIR |= (1<<ELCDPin);
	FIO0PIN &= ~(1<<RSLCDPin);
	FIO0PIN &= ~(1<<ELCDPin);

	// configure data signals (DB0-DB7) as outputs to LCD
	// we configure mpc io expander to set as output already
	setup_mcp();

	// wait 4 ms
	wait_us(4000);
	// write 0x38
	LCDWriteCommand(0x38);
	// write 0x06
	LCDWriteCommand(0x06);
	// write 0x0c
	LCDWriteCommand(0x0c);
	// write 0x01
	LCDWriteCommand(0x01);
	// wait 4 ms
	wait_us(4000);
}

void LCDWriteCommand(int commandCode) {
	// update D0-D7 with command code
	writeToGPIOA(commandCode);
	// drive RS low to indicate command
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
	// drive RS high to indicate data
	FIO0PIN |= (1<<RSLCDPin);
	// drive E high then low (might need short delay)
	FIO0PIN |= (1<<ELCDPin);
	wait_us(10);
	FIO0PIN &= ~(1<<ELCDPin);
	// wait 100 us
	wait_us(100);
}

void LCDWriteString(char string[], int length) {
	for(int i = 0; i < length; ++i) {
		LCDWriteData(string[i]);
	}
}
