# spi_oled: a C library and sample code for Raspberry Pi, for drawing on an SPI-connected OLED panel

## What is this?

spi_oled is a completely self-contained library in C, for drawing on certain
OLED panels with SPI interfaces. It is designed for the 128x128 panel 
made by
Waveshare, based on the SSD1327 controller, but similar panels 
will probably work, perhaps with modification

This library is written in plain C, and requires no other dependencies
except the SPI support built into the Pi Linux kernel. The library specifically
works with SPI, although some OLED panels also have an I2C interface
-- I2C is just too slow for use on a reasonably-sized panel.

## Wiring

The panel's SPI interface has seven connections to the Raspbery Pi:
power and ground, SPI data (MOSI and CLK), and
three additional control lines -- chip select (CS, aka chip enable), 
data/command (DC) and reset (RST). The Waveshare manual shows these
connected to GPIO pins 8, 24, and 25 respectively. Pin 8 is conventional
for SPI devices on Raspberry Pi but, in fact, all these pins are
controlled specifically by the library so, in principle, any three
available GPIO pins could be used. If you're not using pins 8, 24, and
25, modify the relevant values in spi_oled.h 

# Pi setup

To use SPI on the Pi, it needs to be enabled in firmware, and the relevant
kernel modules must be installed. Some Pi Linux distributions will
automatically install the modules if the firmware is enabled, others
require the user to do it.

To enable the firmware, add `dtparam=spi=on` to `config.txt` in the
boot partition. All Pi models require the kernel module 
`spidev`, and most also need a module that is specific to the board, e.g.,
`spi-bcm2835`. 

Programs that use SPI will need to run as `root`, or the permissions
will have to be changed on `/dev/spidevX`.

## Basic operation

The spi\_oled\_xxx methods defined in spi\_oled.h are the client
interface. Initialize an `SPIOled` structure, use it to carry out
drawing operations, and then close it. When closing, you can choose
whether to power off the panel, or leave the contents visible. 

```
spi_oled_debug = FALSE; // Set TRUE if things don't seem to work :)
SPIOled *so = spi_oled_init ("/dev/spidev0.0", 128, 128);
spi_oled_draw_line (so, 80, 50, 50, 50, 1, COLOUR_WHITE);
spi_oled_draw_string (so, 1, 1, &Font20, "Hello", COLOUR_WHITE);
spi_oled_flush (so);
spi_oled_close (so, FALSE); // Leave display on
```

It is important to realize that nothing is written to the display until
spi\_oled\_flush is called. 

The fundamental drawing operation is spi\_oled\_set\_pixel, which can 
be used as the basis for more sophisticated operations.

The `colour` argument taken by many functions is a number between 0 and
15. In monochrome panels this actually sets the brightness (if it does
anything) rather than colour.

## Building

`make` should build the library `libspi_oled.a`. It also builds a test
binary called, unimaginatively, `test`. 

## Fonts

spi-oled text functions use open-source bitmap renditions of Courier fonts, 
whose providers are identified in the relevant source files. There are
five sizes, the number of which fitting into across a 128-pixel OLED are
as follows: 

Font8 -- 5x8 -- 25 characters

Font12 -- 7x12 -- 18 characters

Font16 -- 11x16 -- 11 characters

Font20 -- 14x20 -- 9 characters

Font24 -- 17x24 -- 7 characters 

The fonts are self-weighting, that is, the thickness of the strokes is already
taken care of in the font definitions. You could get a bold effect by
artificially thickening the strokes, but with displays of this low resolution,
and no capability for antialising, this would probably not look good.

Of course, there's nothing to stop you rendering you own fonts on the 
panel, using spi_oled_set_pixel() as a primitive. In principle, it should
even be possible to anti-alias fonts using greyscale values; in practice,
my experience is that there isn't enough greyscale contrast to make that
work very well.

## Notes

You may have to experiment with the clock timing values in spi_oled.c to
find the best compromize between reliability and speed. To some extent
what works here depends on the Pi board itself -- CPU clock speed, etc.

The spi\_oled\_flush() function writes the entire internal frame buffer
to the panel. In principle, it's possible to work out what has changed,
and flush a smaller amount of data. In practice, I'm not at all sure that
the extra calculation involved in doing this is really worth the effort.
At a clock speed of 2Mb/sec, flushing the whole buffer can be done
about 25 times per second.

Flushing the frame buffer once per second, in an application that
does not do any signficant computation between times, will result in
a CPU load of about 0.5%-1%. In most cases, work done by the application
is likely to outweigh the work done in managing and flushing the frame
buffer.




