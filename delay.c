/*
 * delay.c
 *
 *  Created on: Feb 7, 2013
 *      Author: alamer
 */
#include "delay.h"


void delay_ms(uint32_t ms) {  //practical limit of 25,000ms
	uint32_t cl=CLOCK;
    ms = ms * cl / 4;
 while (ms) {
     ms--;
 }
}

void delay_us(uint32_t us) {  //practical limit of 25,000us
	uint32_t cl=CLOCK;
    us = us * cl / (4*1000);
 while (us) {
     us--;
 }
}
