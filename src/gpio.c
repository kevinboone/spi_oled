/*========================================================================
  spi-oled
  gpio.c
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0

  Functions for manipulating the levels of GPIO pins
========================================================================*/
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <spi_oled/gpio.h>
#include <spi_oled/debug.h>

BOOL gpio_set_direction (int pin, GPIODirection direction)
  {
  debug_log ("Call gpio_set_direction: pin=%d, dir=%d", pin, direction);
  char path[512];

  snprintf (path, sizeof (path), "/sys/class/gpio/gpio%d/direction", pin);
  int fd = open (path, O_WRONLY);
  if (fd < 0) 
    {
    debug_log ("Can't open '%s' for writing: %s", path, strerror (errno));
    return FALSE;
    }
   
  if (direction == GPIO_OUT)
    {
    write (fd, "out", 3);
    debug_log ("Pin %d direction set to out", pin);
    }
  else
    {
    write (fd, "in", 2);
    debug_log ("Pin %d direction set to in", pin);
    }

  close (fd);
  return TRUE;
  }


BOOL gpio_export (int pin)
  {
  debug_log ("Call gpio_export: pin=%d", pin);
  int fd = open("/sys/class/gpio/export", O_WRONLY);
  if (fd < 0) 
    {
    debug_log ("Can't export GPIO pin %d: %s", strerror (errno));
    return FALSE;
    }

  char buffer[512];
  int len = snprintf (buffer, sizeof (buffer), "%d", pin);
  write(fd, buffer, len);
    
  debug_log ("Exported GPIO pin %d", pin);

  close (fd);
  return TRUE;
  }


BOOL gpio_set_pin (int pin, GPIOLevel level)
  {
  debug_log ("Call gpio_set_pin: pin=%d, level=%d", pin, level);
  char path[512];

  snprintf (path, sizeof (path), "/sys/class/gpio/gpio%d/value", pin);
  int fd = open (path, O_WRONLY);
  if (fd < 0) 
    {
    debug_log ("can't open %s: %s", path, strerror (errno)); 
    return FALSE;
    }

  if (level == GPIO_HIGH)
    {
    write (fd, "1", 1); 
    debug_log ("Set pin %d high", pin);
    }
  else
    {
    write (fd, "0", 1); 
    debug_log ("Set pin %d low", pin);
    }

  close (fd);
  return TRUE;
  }



