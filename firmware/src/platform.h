/**
 * Platform Abstraction Layer
 * Platform-specific implementations must provide these functions
 */

#ifndef PLATFORM_H
#define PLATFORM_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* SPI Flash */
bool platform_spi_init(void);
bool platform_spi_read(uint32_t offset, uint8_t *buffer, size_t size);
bool platform_spi_write(uint32_t offset, const uint8_t *buffer, size_t size);
bool platform_spi_erase(uint32_t offset);
bool platform_spi_lock(void);
bool platform_spi_unlock(void);
uint32_t platform_spi_get_size(void);

/* USB Mass Storage */
bool platform_usb_init(void);
bool platform_usb_is_present(void);
bool platform_usb_read_file(const char *path, uint8_t *buffer, size_t *size);
bool platform_usb_write_file(const char *path, const uint8_t *buffer, size_t size);
bool platform_usb_delete_file(const char *path);
bool platform_usb_file_exists(const char *path);
bool platform_usb_rename_file(const char *old_path, const char *new_path);

/* Boot Detection */
void platform_boot_detection_init(void);

/* Crypto */
bool platform_crypto_init(void);
void platform_sha256(const uint8_t *data, size_t size, uint8_t *hash);
bool platform_sign(const uint8_t *data, size_t size, 
                  uint8_t *signature, size_t *sig_size);
bool platform_verify(const uint8_t *data, size_t size, 
                    const uint8_t *signature, size_t sig_size);

/* System */
uint32_t platform_get_timestamp(void);
void system_reboot(void);
void src_enter_safe_mode(void);
bool platform_authenticate(void);
void platform_debug_log(const char *message);
void platform_init(void);
void platform_delay_ms(uint32_t ms);

#endif /* PLATFORM_H */
