/*
 * I2C.h
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#ifndef I2C_H_
#define I2C_H_

#define I2C0CONSET (*(volatile unsigned int *)0x4001C000)
#define I2C0STAT (*(volatile unsigned int *)0x4001C004)
#define I2C0DAT (*(volatile unsigned int *)0x4001C008)
#define I2C0SCLH (*(volatile unsigned int *)0x4001C010)
#define I2C0SCLL (*(volatile unsigned int *)0x4001C014)
#define I2C0CONCLR (*(volatile unsigned int *)0x4001C018)

// MCP23017 I/O expander definitions
#define MCP_OPCODE (volatile unsigned int)0b0100000
#define MCP_GPIOA (volatile unsigned int)0x12
#define MCP_IODIRA (volatile unsigned int)0x00

// READ/WRITE bit constants
#define WRITE (volatile unsigned int)0
#define READ (volatile unsigned int)1

void start();

void stop();

void write(int data);

void writeToGPIOA(int data);

void setup_lpc_for_I2C();

void setup_mcp();

#endif /* I2C_H_ */
