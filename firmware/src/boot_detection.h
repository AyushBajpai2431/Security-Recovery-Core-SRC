/**
 * Boot Detection System
 * Multiple redundant methods to detect successful boot
 */

#ifndef BOOT_DETECTION_H
#define BOOT_DETECTION_H

#include <stdint.h>
#include <stdbool.h>
#include "recovery_core.h"

/* Initialize boot detection system */
void boot_detection_init(void);

/* Get current boot status */
void boot_detection_get_status(boot_status_t *status);

/* Update GPIO signal status */
void boot_detection_set_gpio_signal(bool received);

/* Update watchdog status */
void boot_detection_set_watchdog_cleared(bool cleared);

/* Update POST code */
void boot_detection_set_post_code(uint8_t code);

/* Update firmware flag */
void boot_detection_set_firmware_flag(bool set);

#endif /* BOOT_DETECTION_H */
