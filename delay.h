/*
 * delay.h
 *
 * Delay Library for STM32F
 * Need to set CLOCK!!!
 *
 *  Created on: Feb 7, 2013
 *      Author: alamer
 */

#ifndef DELAY_H_
#define DELAY_H_
#include "stdint.h"
/*
 * Must be defined by user
 */
#define CLOCK 168000;


/**
  * @brief  delay in ms
  * @param  ms - delay in ms
  */
void delay_ms(uint32_t ms);

/**
  * @brief  delay in us
  * @param  us - delay in us
  */
void delay_us(uint32_t us);
#endif /* DELAY_H_ */
