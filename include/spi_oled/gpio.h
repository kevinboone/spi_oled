/*========================================================================
  spi-oled
  gpio.h 
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0
========================================================================*/
#pragma once

#include "defs.h"

typedef enum {GPIO_IN=0, GPIO_OUT} GPIODirection;
typedef enum {GPIO_LOW=0, GPIO_HIGH} GPIOLevel;

#ifdef __cplusplus
extern "C" {
#endif

BOOL gpio_export (int pin);
BOOL gpio_set_direction (int pin, GPIODirection direction);
BOOL gpio_set_pin (int pin, GPIOLevel level);

#ifdef __clplusplus
}
#endif


