/*
 * DHT22.h
 *
 * DHT22 library for STM32F
 *  Created on: Feb 6, 2013
 *      Author: alamer
 */

#ifndef DHT22_H_

#define DHT22_H_
#include "stm32f4xx.h"
#include "delay.h"

#define dht22Res_no_error 0
#define dht22Res_not_ready 1
#define dht22Res_data_not_received 2
#define dht22Res_not_valid_crc 3

/**
  * @brief  get data from DHT22
  * @param  GPIOx - used register references
  * @param  GPIO_Pin - connected to DHT22 Pin
  * @retval hum - humidity
  * @retval temp - temperature
  */
uint8_t getDHT22Data(GPIO_TypeDef* GPIOx, uint16_t GPIO_Pin, float *hum, float *temp);
void initDHT22(void);
void initLedsDHT22StateIndication(void);

#endif /* DHT22_H_ */
