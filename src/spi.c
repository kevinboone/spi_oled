/*========================================================================
  spi-oled
  spi.c
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0

  Functions for initializing the SPI subsystem, and transferring
  blocks of data
========================================================================*/
#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdint.h>
#include <sys/ioctl.h>
#include <linux/spi/spidev.h>
#include <spi_oled/spi.h>
#include <spi_oled/debug.h>

SPI* spi_open (const char *dev)
  {
  debug_log ("Call spi_open, dev=%s", dev); 
  int fd = open (dev, O_RDWR);
  if (fd > 0)
    {
    SPI *self = malloc (sizeof (SPI));
    self->fd = fd;
    uint8_t bits = 8;
    int ret = ioctl (fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
    if (ret == -1) 
      debug_log ("can't set bits per word on SPI %s: %s", 
        dev, strerror (errno));
    // I am unsure whether a failure to set the bits per word
    //   should be treated as a failure or not -- the default
    //   may be OK
    ret = ioctl (fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
    if (ret == -1) 
      debug_log ("can't set bits per word on SPI %s: %s", 
        dev, strerror (errno));
    
    self->tr.bits_per_word = bits;
    debug_log ("Set %d bits per word", bits);

    if (spi_set_mode (self, SPI_MODE_0) != 0)
      {
      debug_log ("Can't set SPI mode 0");
      spi_close (self);
      return NULL;
      }

    if (spi_set_chip_select (self, SPI_CS_MODE_LOW) != 0)
      {
      debug_log ("Can't set SPI chip select low");
      spi_close (self);
      return NULL;
      }

    /*
    // TODO -- this method always fails for me, although my 
    //   panels seem to work with it 
    if (spi_set_bit_order (self, SPI_BIT_ORDER_LSBFIRST) != 0)
      {
      debug_log ("Can't set SPI bit order");
      spi_close (self);
      return NULL;
      }
    */

    // Some trial-and-error might be required, to find a suitable speed
    if (spi_set_speed (self, 2000000) != 0)
      {
      debug_log ("Can't set SPI speed");
      spi_close (self);
      return NULL;
      }

    if (spi_set_data_interval (self, 0) != 0)
      {
      debug_log ("Can't set data interval");
      spi_close (self);
      return NULL;
      }

    return self;
    } 
  else
    {
    debug_log ("Can't open %s: %s", dev, strerror (errno));
    return NULL;
    }
  return NULL; // Never get here
  }


int spi_set_mode (SPI *self, SPIMode mode)
  {
  debug_log ("Call spi_set_mode, mode=%d", mode);
  self->mode &= 0xFC; // Clear low two bits
  self->mode |= mode; // Write mode to low bits
  if (ioctl (self->fd, SPI_IOC_WR_MODE, &(self->mode)) == -1) 
    {
    debug_log ("SPI_IOC_WR_MODE ioctl failed");
    return -1;
    }
  debug_log ("Set SPI mode to %d", mode);
  return 0;
  }


int spi_set_speed (SPI *self, int speed)
  {
  debug_log ("Call spi_set_speed, speed=%d", speed);

  if (ioctl(self->fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed) == -1) 
    {
    debug_log ("Can't set speed to %d", speed); 
    // Inability to set speed is not fatal, if we can
    //  determine the preset speed
    }

  int s;
  if (ioctl (self->fd, SPI_IOC_RD_MAX_SPEED_HZ, &s) == -1) 
    {
    // We have no idea what speed is set -- this is fatal
    return -1;
    }
  debug_log ("SPI speed is %d", s); 
  self->speed = s;
  self->tr.speed_hz = s;
  return 0;
  }


int spi_set_chip_select (SPI *self, SPIChipSelect cs_mode)
  {
  debug_log ("Call spi_set_chip_select, cs_mode=%d", cs_mode);
  if (cs_mode == SPI_CS_MODE_HIGH)
    {
    self->mode |= SPI_CS_HIGH;
    self->mode &= ~SPI_NO_CS;
    }
  else if (cs_mode == SPI_CS_MODE_LOW)
    {
    self->mode &= ~SPI_CS_HIGH;
    self->mode &= ~SPI_NO_CS;
    }
  else if (cs_mode == SPI_CS_MODE_NONE)
    {
    self->mode |= SPI_NO_CS;
    }

  if (ioctl (self->fd, SPI_IOC_WR_MODE, &(self->mode)) == -1) 
    {
    debug_log ("ioctl() failed in spi_set_cs_mode");
    return -1;
    }

  debug_log ("Set CS mode to %d", cs_mode);
  return 0;
  }


int spi_set_bit_order (SPI *self, SPIBitOrder order) 
  {
  debug_log ("Call spi_set_bit_order, order=%d", order);

  if (order == SPI_BIT_ORDER_LSBFIRST)
    {
    self->mode |= SPI_LSB_FIRST;
    } 
  else if (order == SPI_BIT_ORDER_MSBFIRST)
    {
    self->mode &= ~SPI_LSB_FIRST;
    }

  if (ioctl (self->fd, SPI_IOC_WR_MODE, &(self->mode)) == -1)
    {
    debug_log ("ioctl() failed in spi_set_bit_order: %s", strerror (errno));
    return -1;
    }

  debug_log ("Set SPI bit order to %d", order);
  return 0;
  }


int spi_set_data_interval (SPI *self, int interval) 
  {
  debug_log ("Call spi_set_data_interval, interval=%d", interval);

  self->delay = interval;
  self->tr.delay_usecs = interval;

  debug_log ("Set SPI data interval to %d", interval);
  return 0;
  }


// What does this return? If anything?
int spi_write_byte (SPI *self, uint8_t value)
  {
  uint8_t rbuf[1];
  self->tr.len = 1;
  self->tr.tx_buf = (unsigned long)&value;
  self->tr.rx_buf = (unsigned long)rbuf;

  if (ioctl (self->fd, SPI_IOC_MESSAGE(1), &(self->tr)) < 1)
    {
    debug_log ("ioctl() failed in spi_write_byte");
    return -1;
    }

  return rbuf[0];
  }


int spi_write_bytes (SPI *self, uint8_t *buf, int len)
  {
  self->tr.len = len;
  self->tr.tx_buf =  (unsigned long)buf;
  self->tr.rx_buf =  (unsigned long)buf;

  if (ioctl (self->fd, SPI_IOC_MESSAGE(1), &(self->tr))  < 1 )
    {
    debug_log ("ioctl() failed in spi_write_bytes");
    return -1;
    }

  return 0;
  }


void spi_close (SPI *self)
  {
  debug_log ("Call spi_close"); 
  if (self)
    {
    close (self->fd);
    }
  else
    debug_log ("self is null in spi_close");
  }


