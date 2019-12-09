/*========================================================================
  spi-oled
  spi-oled.c
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0

  This file contains implementations of functions specific to the
  SPI OLED, which mix GPIO and SPI operations. It also contains the
  basic drawing and text operations.
========================================================================*/
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <spi_oled/spi_oled.h>
#include <spi_oled/gpio.h>
#include <spi_oled/debug.h>
#include <spi_oled/spi.h>
#include <spi_oled/fonts.h>

static void spi_oled_delay_msec (int d)
  {
  for (int i = 0; i < d; i++)
    usleep (1000);
  }


static void spi_oled_write_reg (SPIOled *self, uint8_t value)
  {
  debug_log ("Call spi_oled_write_reg, value=%02x", value);
  gpio_set_pin (OLED_DC, GPIO_LOW);
  gpio_set_pin (OLED_CS, GPIO_LOW);
  spi_write_byte (self->spi, value);
  gpio_set_pin (OLED_CS, GPIO_HIGH);
  }


/* I have only the haziest notion of what these register settings
 * do. Some I figured out from other people's code, others by 
 * trial and error 
 */
static void spi_oled_init_reg (SPIOled *self)
  {
  debug_log ("Call spi_oled_init_reg");
  spi_oled_write_reg (self, 0xae);  // turn off 

  spi_oled_write_reg (self, 0x15);  //  set column address
  spi_oled_write_reg (self, 0x00);  //  start column   0
  spi_oled_write_reg (self, 0x7f);  //  end column   127

  spi_oled_write_reg (self, 0x75);  //  set row address
  spi_oled_write_reg (self, 0x00);  //  start row   0
  spi_oled_write_reg (self, 0x7f);  //  end row   127

  spi_oled_write_reg (self, 0x81);  // set contrast control
  spi_oled_write_reg (self, 0x40);

  spi_oled_write_reg (self, 0xa0);  // gment remap
  spi_oled_write_reg (self, 0x51); 

  spi_oled_write_reg (self, 0xa1);  // start line
  spi_oled_write_reg (self, 0x00);

  spi_oled_write_reg (self, 0xa2);  // display offset
  spi_oled_write_reg (self, 0x00);

  spi_oled_write_reg (self, 0xa4);  // rmal display
  spi_oled_write_reg (self, 0xa8);  // set multiplex ratio
  spi_oled_write_reg (self, 0x7f);

  spi_oled_write_reg (self, 0xb1);  // set phase leghth
  spi_oled_write_reg (self, 0xf1);

  spi_oled_write_reg (self, 0xb3);  // set dclk
  spi_oled_write_reg (self, 0x00);  

  spi_oled_write_reg (self, 0xab); 
  spi_oled_write_reg (self, 0x01); 

  spi_oled_write_reg (self, 0xb6);  // set phase leghth
  spi_oled_write_reg (self, 0x0f);

  spi_oled_write_reg (self, 0xbe);
  spi_oled_write_reg (self, 0x04);

  spi_oled_write_reg (self, 0xbc);
  spi_oled_write_reg (self, 0x08);

  spi_oled_write_reg (self, 0xd5);
  spi_oled_write_reg (self, 0x62);

  spi_oled_write_reg (self, 0xfd);
  spi_oled_write_reg (self, 0x12);
  }


void spi_oled_draw_string (SPIOled *self, uint16_t x, uint16_t y, 
    const sFONT *font, const char *s, uint8_t colour)
  {
  int l = strlen (s);
  for (int i = 0; i < l; i++)
    {
    spi_oled_draw_char (self, x, y, font, s[i], colour);
    x += font->Width;
    }
  }

void spi_oled_draw_char (SPIOled *self, uint16_t x, uint16_t y, 
    const sFONT *font, char c, uint8_t colour)
  {
  if (c < ' ' || c > 127) return;

  int char_offset = (c - ' ') * font->Height * 
       (font->Width / 8 + (font->Width % 8 ? 1 : 0));
  const unsigned char *ptr = &font->table[char_offset];

   for (int page = 0; page < font->Height; page++) 
     {
     for (int column = 0; column < font->Width; column++) 
       {
       if (*ptr & (0x80 >>(column % 8)))
         //spi_oled_draw_square (self, x + column, y + page, 1, colour, TRUE);
         spi_oled_set_pixel (self, x + column, y + page, colour);
       if (column % 8 == 7) ptr++;
       }
     if (font->Width % 8 != 0) ptr++;
     }
  }


/*
 *      1
 *    2   3
 *      4
 *    5   7
 *      6
 */     
static void spi_oled_draw_seqment (SPIOled *self, uint16_t x, uint16_t y, 
      uint16_t width, uint16_t height, int thickness, int seg, uint8_t colour)
  {
  int h2 = height / 2;
  if (seg == 1)
    spi_oled_draw_line (self, x + thickness, y, x + width - thickness, y, 
      thickness, colour);
  else if (seg == 2)
    spi_oled_draw_line (self, x, y + thickness, x, y + h2 - thickness, 
      thickness, colour);
  else if (seg == 3)
    spi_oled_draw_line (self, x + width, y + thickness, x + width, 
      y + h2 - thickness, thickness, colour);
  else if (seg == 4)
    spi_oled_draw_line (self, x + thickness, y + h2, 
      x + width - thickness, y + h2, 
        thickness, colour);
  else if (seg == 5)
    spi_oled_draw_line (self, x, y + h2 + thickness, 
      x, y + height - thickness, thickness, colour);
  else if (seg == 6)
    spi_oled_draw_line (self, x + thickness, y + height,
      x + width - thickness, y + height, 
        thickness, colour);
  else if (seg == 7)
    spi_oled_draw_line (self, x + width, y + h2 + thickness,
      x + width, y + height - thickness, 
        thickness, colour);
  }

void spi_oled_draw_7seg_digit (SPIOled *self, uint16_t x, uint16_t y, 
      uint16_t height, int thickness, int val, uint8_t colour)
  {
  int width = height / 2;
  if (val == 0)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 1, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 2, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 3, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 5, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 6, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    } 
  else if (val == 1)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 3, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    }
  else if (val == 2)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 1, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 3, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 4, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 5, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 6, colour);
    }
  else if (val == 3)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 1, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 3, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 4, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 6, colour);
    }
  else if (val == 4)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 2, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 3, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 4, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    }
  else if (val == 5)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 1, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 2, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 4, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 6, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    }
  else if (val == 6)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 1, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 2, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 4, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 5, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 6, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    }
  else if (val == 7)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 1, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 3, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    }
  else if (val == 8)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 1, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 2, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 3, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 4, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 5, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 6, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 8, colour);
    }
  else if (val == 9)
    {
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 1, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 2, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 3, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 4, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 6, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 7, colour);
    spi_oled_draw_seqment (self, x, y, width, height, thickness, 8, colour);
    }
  }


void spi_oled_draw_square (SPIOled *self, uint16_t x1, uint16_t y1, 
      uint16_t length, uint8_t colour, BOOL fill)
  {
  spi_oled_draw_rect (self, x1, y1, x1 + length, y1 + length, colour, fill);
  }


void spi_oled_draw_line (SPIOled *self, uint16_t x1, uint16_t y1, 
      uint16_t x2, uint16_t y2, int thickness, uint8_t colour)
  {
  /*if (x1 > x2)
    {
    int t = x1;
    x1 = x2;
    x2 = t;
    }
  if (y1 > y2)
    {
    int t = y1;
    y1 = y2;
    y2 = t;
    }*/
  uint16_t x = x1;
  uint16_t y = y1;
  int dx = x2 - x1 >= 0 ? x2 - x1: x1 - x2;
  int dy = y2 - y1 <= 0 ? y2 - y1: y1 - y2;

  int xadd = x1 < x2 ? 1 : -1;
  int yadd = y1 < y2 ? 1 : -1;

  int esp = dx + dy;

  for (;;)
    {
    // It's a bit quicker to use set_pixel if the line is only
    //  one pixel thick
    if (thickness == 1)
      spi_oled_set_pixel (self, x, y, colour);
    else
      spi_oled_draw_square (self, x, y, thickness, colour, TRUE);
    if (2 * esp >= dy)
      {
      if (x == x2) break;
      esp += dy;
      x += xadd;
      }
    if (2 * esp <= dx)
      {
      if (y == y2) break;
      esp += dx;
      y += yadd;
      }
    }
  }


void spi_oled_draw_rect (SPIOled *self, uint16_t x1, uint16_t y1, 
      uint16_t x2, uint16_t y2, uint8_t colour, BOOL fill)
  {
  for (int y = y1; y < y2; y++)
    {
    for (int x = x1; x < x2; x++)
      {
      BOOL draw = FALSE;
      if (fill) draw = TRUE;
      else if (x == x1) draw = TRUE;
      else if (x == x2 - 1) draw = TRUE;
      else if (y == y1) draw = TRUE;
      else if (y == y2 - 1) draw = TRUE;
      if (draw)
        {
        spi_oled_set_pixel (self, x, y, colour);
        }
      }
    }
  }


/* This is the most fundamental drawing function -- set a specific pixel to
 * a specific value. All the other functions are built on this one */
void spi_oled_set_pixel (SPIOled *self, uint16_t x, uint16_t y, uint8_t colour)
  {
  if (x >= self->width) return;
  if (y > self->height) return;
  int half = self->page / 2;
  if (x % 2 == 0) 
    {
    //self->buffer [x / 2 + y * half] = 
    //  (colour << 4) | self->buffer [x / 2 + y * half];
    self->buffer [x / 2 + y * half] = 
      (colour << 4) | (self->buffer [x / 2 + y * half] & 0x0F);
    } 
  else 
    {
    //self->buffer [x / 2 + y * half] = 
    //  (colour & 0x0f) | self->buffer [x / 2 + y * half];
    self->buffer [x / 2 + y * half] = 
      (colour & 0x0f) | (self->buffer [x / 2 + y * half] & 0xF0);
    }
  }


static void spi_oled_set_scan_dir (SPIOled *self, SPIOledScanDir dir)
  {
  debug_log ("Call spi_oled_set_scan_dir, dir=%d", dir);
  self->scan_dir = dir;
  if (dir == L2R_U2D || dir == L2R_D2U 
        || dir == R2L_U2D || dir == R2L_D2U) 
    {
    self->column = self->width; 
    self->page = self->height;
    self->x_adjust = 0;
    self->y_adjust = 0;
    } 
  else 
    {
    self->column = self->height; 
    self->page = self->width;
    self->x_adjust = 0;
    self->y_adjust = 0;
    }
  }


void spi_oled_clear (SPIOled* self, uint8_t colour)
  {
  debug_log ("Call spi_oled_clear, colour=%d", colour);
  unsigned int i,m;
  for (i = 0; i < self->page; i++) 
    {
    for (m = 0; m < (self->column / 2); m++) 
      {
      self->buffer [i * (self->column / 2) + m] = colour | (colour << 4);
      }
    }
  }


static void spi_oled_set_window (SPIOled *self, uint16_t xstart, 
        uint16_t ystart, uint16_t xend, uint16_t yend)	
  {
  debug_log ("Call spi_oled_set_window");
  spi_oled_write_reg (self, 0x15);
  spi_oled_write_reg (self, xstart);
  spi_oled_write_reg (self, xend - 1);

  spi_oled_write_reg (self, 0x75);
  spi_oled_write_reg (self, ystart);
  spi_oled_write_reg (self, yend - 1);
  }


void spi_oled_flush (SPIOled* self)
  {
  debug_log ("Call spi_oled_flush");
  if (self->ready)
    {
    int buff_size = self->width * self->height / 2;
    char *saved_buff = malloc (buff_size);
    memcpy (saved_buff, self->buffer, buff_size);
    uint8_t *pBuf = self->buffer; 

    spi_oled_set_window (self, 0, 0, self->column, self->page);

    gpio_set_pin (OLED_DC, GPIO_HIGH);
    gpio_set_pin (OLED_CS, GPIO_LOW);

    for (int page = 0; page < self->page; page++) 
      {
      spi_write_bytes (self->spi, pBuf, self->column / 2);
      pBuf += self->column / 2;
      }
    gpio_set_pin (OLED_CS, GPIO_HIGH);
    memcpy (self->buffer, saved_buff, buff_size);
    free (saved_buff);
    }
  else
    debug_log ("Called spi_oled_flush but panel not ready");
  }


/*=========================================================================
  spi_oled_reset
  What does this do? It seems to turn the panel off, but it doesn't clear
  it, because the original contents comes back when the panel is turned
  on again
=========================================================================*/
void spi_oled_reset (SPIOled *self)
  {
  debug_log ("Call spi_oled_reset");
  gpio_set_pin (OLED_RST, GPIO_HIGH);
  spi_oled_delay_msec (100);
  gpio_set_pin (OLED_RST, GPIO_LOW);
  spi_oled_delay_msec (100);
  gpio_set_pin (OLED_RST, GPIO_HIGH);
  spi_oled_delay_msec (100);
  }



SPIOled *spi_oled_init (const char *dev, int width, int height)
  {
  // Set GPIO pins 8, 24, and 25, corresponding to CS, RST, and DC,
  //   to outputs
  if (!gpio_export (OLED_CS)) return NULL;
  if (!gpio_export (OLED_RST)) return NULL;
  if (!gpio_export (OLED_DC)) return NULL;
  if (!gpio_set_direction (OLED_CS, GPIO_OUT)) return NULL;
  if (!gpio_set_direction (OLED_RST, GPIO_OUT)) return NULL;
  if (!gpio_set_direction (OLED_DC, GPIO_OUT)) return NULL;
  SPI* spi = spi_open (dev);
  if (spi)
    {
    SPIOled *self = malloc (sizeof (SPIOled));
    self->ready = FALSE;
    self->spi = spi; 
    self->width = width;
    self->height = height;
    self->buffer = malloc (self->width / 2 * self->height);
    spi_oled_reset (self); 
    spi_oled_init_reg (self);
    spi_oled_set_scan_dir (self, SCAN_DIR_DFT);
    spi_oled_delay_msec (200);
    self->ready = TRUE;
    spi_oled_write_reg (self, 0xAF); // Turn on panel
    spi_oled_clear (self, COLOUR_BLACK);
    spi_oled_flush (self);

    return self;
    }
  else
    {
    debug_log ("Can't open SPI device %s: %s", dev, strerror (errno));
    return NULL;
    }
  }


void spi_oled_off (SPIOled *self)
  {
  debug_log ("Call spi_oled_off");
  spi_oled_write_reg (self, 0xAE);
  }


void spi_oled_on (SPIOled *self)
  {
  debug_log ("Call spi_oled_on");
  spi_oled_write_reg (self, 0xAF);
  }


void spi_oled_close (SPIOled *self, BOOL panel_off)
  {
  debug_log ("Call spi_oled_close");
  if (self)
    {
    if (self->spi)
      {
      if (panel_off)
        {
	//spi_oled_clear (self, COLOUR_BLACK);
	//spi_oled_flush (self);
        spi_oled_off (self);
        }
      spi_close (self->spi);
      }
    else
      debug_log ("self->spi is null in spi_oled_close");
    if (self->buffer)
      free (self->buffer);
    else
      debug_log ("self->buffer is null in spi_oled_close");
    free (self);
    }
  else
    debug_log ("self is null in spi_oled_close");
  }

