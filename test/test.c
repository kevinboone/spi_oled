/*========================================================================
  spi-oled
  test.c
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0

  This is a test driver for the SPI-OLED library. It just 
  shows the time, date, and CPU temperature on the panel
========================================================================*/
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <spi_oled/spi_oled.h>

#define DEVICE "/dev/spidev0.0"

int get_temp (void)
  {
  FILE *f = fopen ("/sys/class/thermal/thermal_zone0/temp", "r");
  if (f)
    {
    int t = 0;
    fscanf (f, "%d", &t);
    t = t / 1000;
    fclose (f);
    return t;
    }
  else
    return 0; // Well, what else can we do?
  }


int main (int argc, char **argv)
  {
  spi_oled_debug = FALSE; // Set TRUE if things don't seem to work :)
  SPIOled *so = spi_oled_init (DEVICE, 128, 128);
  if (so)
    {
    // Some other possibilities...
    //spi_oled_draw_line (so, 80, 50, 50, 50, 1, COLOUR_WHITE);
    //spi_oled_draw_string (so, 1, 1, &Font20, "Hello", COLOUR_WHITE);
    //spi_oled_draw_string (so, 1, 25, &Font24, "12:35", COLOUR_WHITE);
    //spi_oled_draw_7seg_digit (so, 5, 50, 25, 1, 4, 0x01);
    //spi_oled_draw_7seg_digit (so, 25, 50, 25, 1, 6, 0x01);
    //spi_oled_draw_7seg_digit (so, 45, 50, 25, 1, 9, COLOUR_WHITE);
    //spi_oled_draw_7seg_digit (so, 65, 50, 25, 1, 0, COLOUR_WHITE);
    //spi_oled_flush (so);

    int ticks = 0;
    for (;;)
      {
      time_t now = time (NULL);
      char *tbuff = ctime (&now);
      tbuff[20] = 0;
      tbuff[10] = 0;
      if (ticks == 60)
        {
        ticks = -1;
	}
      else if (ticks == 0)
        {
        // Draw the date
        spi_oled_draw_rect (so, 29, 25, so->width, 25 + 14, 
            COLOUR_BLACK, TRUE);
        spi_oled_draw_string (so, 29, 25, &Font12, tbuff, 3);

	// Draw a grey box with a white border
        spi_oled_draw_rect (so, 20, 44, 20 + 85 + 2, 49 + 25 + 2, 
            COLOUR_WHITE, FALSE);
        spi_oled_draw_rect (so, 21, 45, 21 + 85, 50 + 25, 1, TRUE);

	// Get and draw the temperature
	char temp_string [10];
	int temp = get_temp();
	sprintf (temp_string, "%3d C", temp);
        spi_oled_draw_string (so, 21, 50, &Font24, temp_string, COLOUR_BLACK);
	// Degree sign ;)
        spi_oled_draw_string (so, 80, 50, &Font12, "o", COLOUR_BLACK);
	}

      // Draw the time
      spi_oled_draw_rect (so, 5, 5, so->width, 5 + 16, 0, TRUE);
      spi_oled_draw_string (so, 5, 5, &Font20, tbuff + 11, COLOUR_WHITE);
      spi_oled_flush (so);
      ticks++;
      sleep (1);
      }

    spi_oled_close (so, FALSE);
    }
  else
    fprintf (stderr, "Can't initialize SPI OLED device\n"); 
  return 0;
  }

