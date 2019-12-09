/*========================================================================
  spi-oled
  spi-oled.h 
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0
========================================================================*/
#pragma once

// Define the GPIO pins that will be used to control chip select,
//  reset, and data/command. The default values of 8, 25, and 24
//  correspond to the wiring described in the Waveshare manual
#define OLED_CS   8
#define OLED_RST  25
#define OLED_DC   24

#include "debug.h"
#include "spi.h"
#include "fonts.h"

#define COLOUR_BLACK 0
#define COLOUR_WHITE 0x0F

typedef enum
  {
  L2R_U2D = 0,   // left to right, up to down 
  L2R_D2U,
  R2L_U2D,
  R2L_D2U,
  U2D_L2R,
  U2D_R2L,
  D2U_L2R,
  D2U_R2L,
  } SPIOledScanDir;
#define SCAN_DIR_DFT  L2R_U2D 

typedef struct _SPIOled 
  {
  SPI *spi;
  // Scan direction -- the way in which the frame buffer is read into
  //  the panel using SPI
  SPIOledScanDir scan_dir;
  int width;  // Pixels
  int height; // Pixels
  // column and page represent the way data is stored in the frame buffer,
  //  which can be column-first or row-first
  // These values are set from the scan direction in set_scan_dir() but,
  //  in practice, column=width and page=height
  int column; // Pixels
  int page;   // Pixels
  // In practice, x_adjust and y_adjust seem to be OK at zero
  int x_adjust;
  int y_adjust;
  // buffer is the panel screen buffer. The panel uses a 4-bit value
  //   for each pixel, so the total size of the buffer is 
  //   width * height / 2
  uint8_t *buffer; 
  // ready is set to TRUE when the panel seems to be ready to accept data
  // We use this to determine whether it is safe to update the panel
  BOOL ready;
  } SPIOled;


#ifdef __cplusplus
extern "C" {
#endif


// Initialize the library and the hardware, specifying a panel 
//  width and height. The object returned must be passed to all
//  further panel-related methods
SPIOled *spi_oled_init (const char *dev, int width, int height);

// Close the SPI device, clear memory and, optionally, power off the panel
void spi_oled_close (SPIOled *self, BOOL panel_off);

// Fill the frame buffer with the specified colour, usually COLOUR_BLACK. 
// Nothing is written to the panel until flush() is called
void spi_oled_clear (SPIOled* self, uint8_t colour);

// Set the pixel at the specified point. Nothing is written to the 
// panel until flush() is called
void spi_oled_set_pixel (SPIOled *self, uint16_t x, uint16_t y, 
  uint8_t colour);

// Flush the entire framebuffer to the panel. Note that noe of the 
//  other drawing methods have any effect on the display until flush()
//  is called
void spi_oled_flush (SPIOled* self);

// Turn the panel on. Any data that was previous written remains in place,
//  unless the panel is specifically cleared
void spi_oled_on (SPIOled *self);

// Turn the panel off. Does not clear any data, either in this object or
//  in the panel itself 
void spi_oled_off (SPIOled *self);

// Draw a rectangle with the specified corners. Note that the figure
//  includes x1,y1 buts does _not_ extend to x2,y2
void spi_oled_draw_rect (SPIOled *self, uint16_t x1, uint16_t y1, 
      uint16_t x2, uint16_t y2, uint8_t colour, BOOL fill);

// Draw a square of specified length with corner x1,y1. Note that the
// length is _exclusive_ -- that is, the length gives the number of
// pixels on a side, not the end pixel
void spi_oled_draw_square (SPIOled *self, uint16_t x1, uint16_t y1, 
      uint16_t length, uint8_t colour, BOOL fill);

// Draw a single ASCII character, whose top-left corner is x,y
void spi_oled_draw_char (SPIOled *self, uint16_t x, uint16_t y, 
    const sFONT *font, char c, uint8_t colour);

// Draw a string of ASCII characters, whose top-left corner is x,y
// Note that there is no wrapping -- text that will not fit the
//  screen is truncated
void spi_oled_draw_string (SPIOled *self, uint16_t x, uint16_t y, 
    const sFONT *font, const char *s, uint8_t colour);

// Draw a 7-segment-style digit with top-left corner x,y, or specified
//  width, heigh, and thickness. Note that this function can only
//  draw digits, and the 'val' argument is a number between zero and
//  one, not a character
void spi_oled_draw_7seg_digit (SPIOled *self, uint16_t x, uint16_t y, 
      uint16_t height, int thickness, int val, uint8_t colour);

// Draw a line of specified pixel thickness between x1,x2 and y1,y2
void spi_oled_draw_line (SPIOled *self, uint16_t x1, uint16_t y1, 
      uint16_t x2, uint16_t y2, int thickness, uint8_t colour);

#ifdef __clplusplus
}
#endif


