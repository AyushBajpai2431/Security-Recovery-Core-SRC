/**
 * Security Recovery Core (SRC) - Firmware Header
 * 
 * Recovery Core runs before BIOS/UEFI to provide hardware-assisted
 * firmware recovery capabilities.
 */

#ifndef RECOVERY_CORE_H
#define RECOVERY_CORE_H

#include <stdint.h>
#include <stdbool.h>

/* Version Information */
#define SRC_VERSION_MAJOR 1
#define SRC_VERSION_MINOR 0
#define SRC_VERSION_PATCH 0

/* Configuration */
#define MAX_BACKUP_INTERVAL_MS (10 * 60 * 1000)  // 10 minutes
#define BOOT_TIMEOUT_MS (30000)                   // 30 seconds
#define MAX_DISABLE_DURATION_MS (7 * 24 * 60 * 60 * 1000)  // 7 days max

/* SPI Flash Layout */
#define SPI_FLASH_SIZE (16 * 1024 * 1024)  // 16MB typical
#define SRC_RESERVED_REGION_START (0x100000)  // 1MB offset
#define SRC_RESERVED_REGION_SIZE (512 * 1024)  // 512KB for SRC
#define FIRMWARE_REGION_START (0x0)
#define FIRMWARE_REGION_SIZE (8 * 1024 * 1024)  // 8MB for main firmware

/* USB Recovery Path */
#define USB_RECOVERY_PATH "/SECURITY_RECOVERY"
#define BACKUP_A_FILE "A.bin"
#define BACKUP_B_FILE "B.bin"
#define MANIFEST_FILE "manifest.json"
#define SIGNATURE_FILE "signature.sig"
#define METADATA_FILE "metadata.txt"

/* State Machine States */
typedef enum {
    SRC_STATE_INIT,
    SRC_STATE_CHECKING_BOOT,
    SRC_STATE_BOOT_SUCCESS,
    SRC_STATE_BOOT_FAILED,
    SRC_STATE_RECOVERING,
    SRC_STATE_BACKUP_ACTIVE,
    SRC_STATE_DISABLED,
    SRC_STATE_REMOVING
} src_state_t;

/* Boot Detection Methods */
typedef struct {
    bool gpio_signal_received;
    bool watchdog_cleared;
    uint8_t post_code;
    bool firmware_flag_set;
    uint32_t timestamp;
} boot_status_t;

/* SRC Configuration */
typedef struct {
    bool enabled;
    uint32_t disable_until_timestamp;  // 0 = not disabled
    uint32_t last_backup_timestamp;
    uint32_t last_recovery_timestamp;
    char board_id[32];
    uint8_t firmware_hash[32];  // SHA-256
} src_config_t;

/* Function Prototypes */

/**
 * Initialize Recovery Core
 * Called at power-on, before any other firmware initialization
 */
void src_init(void);

/**
 * Main state machine loop
 * Must be called periodically from main loop
 */
void src_main_loop(void);

/**
 * Check if boot was successful using multiple methods
 */
bool src_check_boot_success(void);

/**
 * Attempt recovery from USB device
 */
bool src_recover_from_usb(void);

/**
 * Perform automatic backup if conditions are met
 */
void src_perform_backup(void);

/**
 * Check if recovery core is currently disabled
 */
bool src_is_disabled(void);

/**
 * Disable recovery core temporarily (time-based)
 */
bool src_disable_temporary(uint32_t duration_ms);

/**
 * Enable recovery core
 */
bool src_enable(void);

/**
 * Schedule removal of recovery core
 */
bool src_schedule_removal(void);

/**
 * Verify cryptographic signature of firmware image
 */
bool src_verify_signature(const uint8_t *firmware, size_t size, 
                         const uint8_t *signature, size_t sig_size);

/**
 * Read configuration from SPI flash
 */
bool src_read_config(src_config_t *config);

/**
 * Write configuration to SPI flash
 */
bool src_write_config(const src_config_t *config);

/**
 * Read firmware from SPI flash
 */
bool src_read_firmware(uint8_t *buffer, size_t size, uint32_t offset);

/**
 * Write firmware to SPI flash with verification
 */
bool src_write_firmware(const uint8_t *buffer, size_t size, uint32_t offset);

/**
 * Initialize USB Mass Storage interface
 */
bool src_usb_init(void);

/**
 * Check if USB device is present and valid
 */
bool src_usb_check_present(void);

/**
 * Read file from USB device
 */
bool src_usb_read_file(const char *path, uint8_t *buffer, size_t *size);

/**
 * Write file to USB device
 */
bool src_usb_write_file(const char *path, const uint8_t *buffer, size_t size);

/**
 * Log message (tamper-resistant)
 */
void src_log(const char *message, ...);

#endif /* RECOVERY_CORE_H */
