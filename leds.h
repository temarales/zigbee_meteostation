/// \file leds.h  
/// Library for working with stm32.

#ifndef _LIBRARY_H_
#define _LIBRARY_H_

#include <stdint.h>
#include "stm32f4xx.h"
#include "stm32f4xx_rcc.h"
#include "stm32f4xx_tim.h"
#include "stm32f4xx_gpio.h"
#include "stm32f4xx_exti.h"
#include "stm32f4xx_syscfg.h"

/// \brief Color setting(RGB).
/// \param[in] r Brightness of red: 0..255
/// \param[in] g Brightness of green: 0..255
/// \param[in] b Brightness of blue: 0..255
void set_color(double r, double g, double b);

/// \brief Switch color to one of a limited set.
/// \param[in] num Color number in set
void switch_color(int num);

/// \brief Start TIM1.
void start_TIM1(void);

/// \brief Initialize interractions for buttons.
void init_buttons_interractions(void);

/// \brief Initialize buttons.
void init_buttons(void);

/// \brief Initialize leds with alternative function.
void init_leds_AF_TIM1(void);

/// \brief Set brigthness to level
/// \param[in] level Level of brightness from 0 to 1.
void set_brightness(double level);

#endif
