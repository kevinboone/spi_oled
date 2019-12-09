/*========================================================================
  spi-oled
  debug.h
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0
========================================================================*/
#pragma once

#include "defs.h"

#ifdef __cplusplus
extern "C" {
#endif

extern BOOL spi_oled_debug;
void        debug_log (const char *fmt,...);

#ifdef __clplusplus
}
#endif

