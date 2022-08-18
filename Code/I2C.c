/*
 * I2C.c
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#include "GeneralLPC.h"
#include "I2C.h"

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
 * Sets up microcontroller for I2C operation
 */
void setup_lpc_for_I2C() {
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
	// configure GPIOA as output
	start();
	write((MCP_OPCODE<<1) | (WRITE<<0));
	write(MCP_IODIRA);
	write(0x00);
	stop();
}
