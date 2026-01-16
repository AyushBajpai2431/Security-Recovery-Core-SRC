/**
 * SPI Flash Interface
 * Provides low-level access to SPI flash memory
 */

#ifndef SPI_FLASH_H
#define SPI_FLASH_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Initialize SPI flash interface */
bool spi_flash_init(void);

/* Read data from SPI flash */
bool spi_flash_read(uint32_t offset, uint8_t *buffer, size_t size);

/* Write data to SPI flash */
bool spi_flash_write(uint32_t offset, const uint8_t *buffer, size_t size);

/* Erase sector (typically 4KB or 64KB) */
bool spi_flash_erase_sector(uint32_t offset);

/* Lock SPI flash (hardware protection) */
bool spi_flash_lock(void);

/* Unlock SPI flash */
bool spi_flash_unlock(void);

/* Get flash size */
uint32_t spi_flash_get_size(void);

#endif /* SPI_FLASH_H */
