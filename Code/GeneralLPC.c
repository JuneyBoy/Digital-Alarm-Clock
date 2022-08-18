/*
 * GeneralLPC.c
 *
 *  Created on: Nov 18, 2021
 *      Author: Arjun Ganesan and Daniel Ngo
 */

#include "GeneralLPC.h"
#include "I2C.h"

//functions taken from Timer Lecture
void wait_us(int us){
	int start = T0TC;  // note starting time
	T0TCR |= (1<<0);   // start timer
	while ((T0TC-start)<us) {} // wait for time to pass
}
void wait(float sec){
	wait_us(sec*1000000); // convert seconds to microseconds
}
