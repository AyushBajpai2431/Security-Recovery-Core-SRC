/**
 * SPI Flash Implementation
 * Platform-specific implementations should override these functions
 */

#include "spi_flash.h"
#include "platform.h"
#include <string.h>

static bool spi_initialized = false;

bool spi_flash_init(void) {
    if (spi_initialized) {
        return true;
    }
    
    /* Platform-specific initialization */
    if (!platform_spi_init()) {
        return false;
    }
    
    spi_initialized = true;
    return true;
}

bool spi_flash_read(uint32_t offset, uint8_t *buffer, size_t size) {
    if (!spi_initialized || !buffer || size == 0) {
        return false;
    }
    
    /* Platform-specific read */
    return platform_spi_read(offset, buffer, size);
}

bool spi_flash_write(uint32_t offset, const uint8_t *buffer, size_t size) {
    if (!spi_initialized || !buffer || size == 0) {
        return false;
    }
    
    /* Erase sector if needed before write */
    uint32_t sector_size = 4096;  // Typical 4KB sectors
    uint32_t sector_start = (offset / sector_size) * sector_size;
    
    if (offset % sector_size == 0) {
        if (!spi_flash_erase_sector(sector_start)) {
            return false;
        }
    }
    
    /* Platform-specific write */
    return platform_spi_write(offset, buffer, size);
}

bool spi_flash_erase_sector(uint32_t offset) {
    if (!spi_initialized) {
        return false;
    }
    
    /* Platform-specific erase */
    return platform_spi_erase(offset);
}

bool spi_flash_lock(void) {
    if (!spi_initialized) {
        return false;
    }
    
    /* Platform-specific lock */
    return platform_spi_lock();
}

bool spi_flash_unlock(void) {
    if (!spi_initialized) {
        return false;
    }
    
    /* Platform-specific unlock */
    return platform_spi_unlock();
}

uint32_t spi_flash_get_size(void) {
    /* Platform-specific size detection */
    return platform_spi_get_size();
}
