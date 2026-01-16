/**
 * Boot Detection Implementation
 */

#include "boot_detection.h"
#include "platform.h"
#include <string.h>

static boot_status_t boot_status = {0};

void boot_detection_init(void) {
    memset(&boot_status, 0, sizeof(boot_status));
    boot_status.timestamp = platform_get_timestamp();
    
    /* Initialize platform-specific boot detection */
    platform_boot_detection_init();
}

void boot_detection_get_status(boot_status_t *status) {
    if (status) {
        memcpy(status, &boot_status, sizeof(boot_status_t));
    }
}

void boot_detection_set_gpio_signal(bool received) {
    boot_status.gpio_signal_received = received;
    boot_status.timestamp = platform_get_timestamp();
}

void boot_detection_set_watchdog_cleared(bool cleared) {
    boot_status.watchdog_cleared = cleared;
    boot_status.timestamp = platform_get_timestamp();
}

void boot_detection_set_post_code(uint8_t code) {
    boot_status.post_code = code;
    boot_status.timestamp = platform_get_timestamp();
}

void boot_detection_set_firmware_flag(bool set) {
    boot_status.firmware_flag_set = set;
    boot_status.timestamp = platform_get_timestamp();
}
