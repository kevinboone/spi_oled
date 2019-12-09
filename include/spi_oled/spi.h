/*========================================================================
  spi-oled
  spi.h 
  Copyright (c)2019-20 Kevin Boone
  Distributed under the terms of the GPL, v3.0
========================================================================*/
#pragma once

#include <linux/spi/spidev.h>
#include <stdint.h>
#include "defs.h"

typedef enum 
  {
  SPI_MODE0 = SPI_MODE_0,  /*!< CPOL = 0, CPHA = 0 */
  SPI_MODE1 = SPI_MODE_1,  /*!< CPOL = 0, CPHA = 1 */
  SPI_MODE2 = SPI_MODE_2,  /*!< CPOL = 1, CPHA = 0 */
  SPI_MODE3 = SPI_MODE_3   /*!< CPOL = 1, CPHA = 1 */
  } SPIMode;


typedef enum
  {
  SPI_CS_MODE_LOW  = 0,     /*!< Chip Select 0 */
  SPI_CS_MODE_HIGH = 1,     /*!< Chip Select 1 */
  SPI_CS_MODE_NONE = 3      /*!< No CS */
  } SPIChipSelect;


typedef enum
  {
  SPI_BIT_ORDER_LSBFIRST = 0,  /*!< LSB First */
  SPI_BIT_ORDER_MSBFIRST = 1   /*!< MSB First */
  } SPIBitOrder;


typedef struct _SPI 
  {
  int fd;
  uint16_t mode;
  int speed; // bits per sec
  int delay; // In usec
  struct spi_ioc_transfer tr;
  } SPI;

#ifdef __cplusplus
extern "C" {
#endif

SPI *spi_open (const char *dev);
void spi_close (SPI *spi);

int spi_set_mode (SPI *self, SPIMode mode);
int spi_set_chip_select (SPI *self, SPIChipSelect cs_mode);
int spi_set_bit_order (SPI *self, SPIBitOrder order);
int spi_set_speed (SPI *self, int speed);
int spi_set_speed (SPI *self, int speed);
int spi_set_data_interval (SPI *self, int interval);
int spi_write_byte (SPI *self, uint8_t value);
int spi_write_bytes (SPI *self, uint8_t *buf, int n);

#ifdef __clplusplus
}
#endif


