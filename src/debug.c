/*========================================================================
  spi-oled
  debug.c
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0

  This file primarily contains the implementation of debug_log(), which
  is called by many of the libraries methods during initialization
========================================================================*/
#include <stdarg.h>
#include <stdlib.h>
#include <stdio.h>
#include <spi_oled/debug.h>

BOOL spi_oled_debug = FALSE;

static char *debug_format_args (const char *fmt, va_list ap)
  {
  int n;
  int size = 100;
  char *p, *np;

  if ((p = malloc(size)) == NULL)
    return NULL;

  while (1)
    {
    n = vsnprintf(p, size, fmt, ap);

    if (n > -1 && n < size)
       return p;

    if (n > -1)
      size = n+1;
    else
      size *= 2;

   if ((np = realloc (p, size)) == NULL)
      {
      free(p);
        return NULL;
      }
    else
      {
      p = np;
      }
    }
  return NULL; // Never get here 
  }


static void debug_log_v (const char *fmt, va_list ap)
  {
  char *s = debug_format_args (fmt, ap);
  printf ("%s\n", s);
  free (s);
  }

void debug_log (const char *fmt, ...)
  {
  if (!spi_oled_debug) return;
  va_list ap;
  va_start (ap, fmt);
  debug_log_v (fmt, ap);
  va_end (ap);
  }

