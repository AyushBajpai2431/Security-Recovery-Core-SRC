/**
 * Security Recovery Core (SRC) - Main Implementation
 * 
 * This firmware runs before BIOS/UEFI to provide hardware-assisted recovery.
 */

#include "recovery_core.h"
#include "spi_flash.h"
#include "usb_msd.h"
#include "boot_detection.h"
#include "crypto.h"
#include "logging.h"
#include "platform.h"
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

/* Global State */
static src_state_t current_state = SRC_STATE_INIT;
static src_config_t config;
static boot_status_t boot_status;
static uint32_t boot_start_timestamp = 0;
static bool removal_scheduled = false;

/**
 * Initialize Recovery Core
 */
void src_init(void) {
    src_log("SRC: Initializing Recovery Core v%d.%d.%d",
            SRC_VERSION_MAJOR, SRC_VERSION_MINOR, SRC_VERSION_PATCH);
    
    /* Initialize hardware interfaces */
    if (!spi_flash_init()) {
        src_log("SRC: ERROR - SPI flash initialization failed");
        return;
    }
    
    if (!crypto_init()) {
        src_log("SRC: ERROR - Crypto initialization failed");
        return;
    }
    
    if (!logging_init()) {
        src_log("SRC: WARNING - Logging initialization failed");
    }
    
    if (!src_usb_init()) {
        src_log("SRC: WARNING - USB initialization failed (may not be present)");
    }
    
    /* Read configuration from SPI flash */
    if (!src_read_config(&config)) {
        src_log("SRC: No existing config found, initializing defaults");
        memset(&config, 0, sizeof(config));
        config.enabled = true;
        strncpy(config.board_id, "DEFAULT", sizeof(config.board_id) - 1);
    }
    
    /* Check if removal is scheduled (read from config) */
    /* In production, this would be stored in a separate flag */
    if (removal_scheduled) {
        current_state = SRC_STATE_REMOVING;
        src_log("SRC: Removal scheduled, entering removal state");
        return;
    }
    
    /* Check if temporarily disabled */
    if (src_is_disabled()) {
        current_state = SRC_STATE_DISABLED;
        uint32_t remaining = config.disable_until_timestamp - src_get_timestamp();
        src_log("SRC: Temporarily disabled, %lu ms remaining", remaining);
        return;
    }
    
    if (!config.enabled) {
        current_state = SRC_STATE_DISABLED;
        src_log("SRC: Recovery core is disabled");
        return;
    }
    
    /* Initialize boot detection */
    boot_detection_init();
    boot_start_timestamp = platform_get_timestamp();
    current_state = SRC_STATE_CHECKING_BOOT;
    
    src_log("SRC: Initialization complete, monitoring boot");
}

/**
 * Main state machine loop
 */
void src_main_loop(void) {
    switch (current_state) {
        case SRC_STATE_INIT:
            src_init();
            break;
            
        case SRC_STATE_CHECKING_BOOT:
            /* Check if boot timeout has elapsed */
            if ((platform_get_timestamp() - boot_start_timestamp) > BOOT_TIMEOUT_MS) {
                src_log("SRC: Boot timeout exceeded, boot considered failed");
                current_state = SRC_STATE_BOOT_FAILED;
            } else if (src_check_boot_success()) {
                src_log("SRC: Boot success detected");
                current_state = SRC_STATE_BOOT_SUCCESS;
            }
            break;
            
        case SRC_STATE_BOOT_SUCCESS:
            /* System booted successfully, perform backup if needed */
            src_perform_backup();
            /* Transition to monitoring state */
            current_state = SRC_STATE_BACKUP_ACTIVE;
            break;
            
        case SRC_STATE_BOOT_FAILED:
            /* Boot failed, attempt recovery */
            src_log("SRC: Boot failure detected, attempting recovery");
            if (src_recover_from_usb()) {
                src_log("SRC: Recovery successful, rebooting");
                /* Trigger system reboot */
                system_reboot();
            } else {
                src_log("SRC: Recovery failed, system may be bricked");
                /* Enter safe mode - allow manual intervention */
                src_enter_safe_mode();
            }
            break;
            
        case SRC_STATE_RECOVERING:
            /* Recovery in progress, wait for completion */
            break;
            
        case SRC_STATE_BACKUP_ACTIVE:
            /* System is healthy, perform periodic backups */
            src_perform_backup();
            break;
            
        case SRC_STATE_DISABLED:
            /* Recovery core is disabled, check if re-enable time has passed */
            if (src_is_disabled() && config.disable_until_timestamp > 0) {
                uint32_t now = platform_get_timestamp();
                if (now >= config.disable_until_timestamp) {
                    src_log("SRC: Disable period expired, re-enabling");
                    config.disable_until_timestamp = 0;
                    src_write_config(&config);
                    current_state = SRC_STATE_CHECKING_BOOT;
                    boot_start_timestamp = platform_get_timestamp();
                }
            }
            break;
            
        case SRC_STATE_REMOVING:
            /* Handle removal process */
            src_handle_removal();
            break;
    }
}

/**
 * Check if boot was successful
 */
bool src_check_boot_success(void) {
    boot_status_t status;
    boot_detection_get_status(&status);
    
    /* Boot is successful if ANY of these conditions are met */
    if (status.gpio_signal_received) {
        src_log("SRC: Boot success - GPIO signal received");
        return true;
    }
    
    if (status.watchdog_cleared) {
        src_log("SRC: Boot success - Watchdog cleared");
        return true;
    }
    
    if (status.post_code >= 0xA0) {  // POST code threshold
        src_log("SRC: Boot success - POST code 0x%02X", status.post_code);
        return true;
    }
    
    if (status.firmware_flag_set) {
        src_log("SRC: Boot success - Firmware flag set");
        return true;
    }
    
    return false;
}

/**
 * Attempt recovery from USB device
 */
bool src_recover_from_usb(void) {
    src_log("SRC: Starting USB recovery process");
    
    if (!src_usb_check_present()) {
        src_log("SRC: ERROR - USB device not present");
        return false;
    }
    
    current_state = SRC_STATE_RECOVERING;
    
    /* Read manifest to determine which backup to use */
    uint8_t manifest_buffer[4096];
    size_t manifest_size = sizeof(manifest_buffer);
    
    if (!src_usb_read_file("/SECURITY_RECOVERY/manifest.json", 
                           manifest_buffer, &manifest_size)) {
        src_log("SRC: ERROR - Cannot read manifest.json");
        return false;
    }
    
    /* Parse manifest (simplified - in production use proper JSON parser) */
    /* For now, try backup A first, then B */
    const char *backup_files[] = {BACKUP_A_FILE, BACKUP_B_FILE};
    bool recovery_success = false;
    
    for (int i = 0; i < 2; i++) {
        src_log("SRC: Attempting recovery from %s", backup_files[i]);
        
        char backup_path[256];
        snprintf(backup_path, sizeof(backup_path), 
                "/SECURITY_RECOVERY/%s", backup_files[i]);
        
        /* Read firmware image */
        uint8_t *firmware_buffer = malloc(FIRMWARE_REGION_SIZE);
        if (!firmware_buffer) {
            src_log("SRC: ERROR - Memory allocation failed");
            continue;
        }
        
        size_t firmware_size = FIRMWARE_REGION_SIZE;
        if (!src_usb_read_file(backup_path, firmware_buffer, &firmware_size)) {
            src_log("SRC: ERROR - Cannot read %s", backup_files[i]);
            free(firmware_buffer);
            continue;
        }
        
        /* Read signature */
        char sig_path[256];
        snprintf(sig_path, sizeof(sig_path), 
                "/SECURITY_RECOVERY/signature.sig", backup_files[i]);
        
        uint8_t signature[512];
        size_t sig_size = sizeof(signature);
        if (!src_usb_read_file(sig_path, signature, &sig_size)) {
            src_log("SRC: ERROR - Cannot read signature");
            free(firmware_buffer);
            continue;
        }
        
        /* Verify signature */
        if (!src_verify_signature(firmware_buffer, firmware_size, 
                                  signature, sig_size)) {
            src_log("SRC: ERROR - Signature verification failed for %s", 
                   backup_files[i]);
            free(firmware_buffer);
            continue;
        }
        
        /* Write firmware to SPI flash */
        if (!src_write_firmware(firmware_buffer, firmware_size, 
                               FIRMWARE_REGION_START)) {
            src_log("SRC: ERROR - Failed to write firmware to SPI");
            free(firmware_buffer);
            continue;
        }
        
        src_log("SRC: Successfully recovered from %s", backup_files[i]);
        recovery_success = true;
        config.last_recovery_timestamp = platform_get_timestamp();
        src_write_config(&config);
        
        free(firmware_buffer);
        break;
    }
    
    current_state = SRC_STATE_CHECKING_BOOT;
    return recovery_success;
}

/**
 * Perform automatic backup if conditions are met
 */
void src_perform_backup(void) {
    if (!config.enabled || src_is_disabled()) {
        return;
    }
    
    /* Check if backup interval has elapsed */
    uint32_t now = platform_get_timestamp();
    uint32_t time_since_backup = now - config.last_backup_timestamp;
    
    if (time_since_backup < MAX_BACKUP_INTERVAL_MS) {
        return;  // Too soon for next backup
    }
    
    if (!src_usb_check_present()) {
        src_log("SRC: USB not present, skipping backup");
        return;
    }
    
    src_log("SRC: Starting automatic backup");
    
    /* Read current firmware */
    uint8_t *firmware_buffer = malloc(FIRMWARE_REGION_SIZE);
    if (!firmware_buffer) {
        src_log("SRC: ERROR - Memory allocation failed");
        return;
    }
    
    if (!src_read_firmware(firmware_buffer, FIRMWARE_REGION_SIZE, 
                          FIRMWARE_REGION_START)) {
        src_log("SRC: ERROR - Failed to read firmware");
        free(firmware_buffer);
        return;
    }
    
    /* Calculate hash */
    uint8_t hash[32];
    crypto_sha256(firmware_buffer, FIRMWARE_REGION_SIZE, hash);
    
    /* Check if firmware has changed */
    if (memcmp(hash, config.firmware_hash, 32) == 0) {
        src_log("SRC: Firmware unchanged, skipping backup");
        free(firmware_buffer);
        return;
    }
    
    /* Rotate backups: B -> deleted, A -> B, new -> A */
    /* First, delete old B if it exists */
    char backup_b_path[256];
    snprintf(backup_b_path, sizeof(backup_b_path), 
            "/SECURITY_RECOVERY/%s", BACKUP_B_FILE);
    src_usb_delete_file(backup_b_path);
    
    /* Move A to B */
    char backup_a_path[256];
    snprintf(backup_a_path, sizeof(backup_a_path), 
            "/SECURITY_RECOVERY/%s", BACKUP_A_FILE);
    
    if (src_usb_file_exists(backup_a_path)) {
        src_usb_rename_file(backup_a_path, backup_b_path);
    }
    
    /* Write new firmware to A */
    if (!src_usb_write_file(backup_a_path, firmware_buffer, 
                           FIRMWARE_REGION_SIZE)) {
        src_log("SRC: ERROR - Failed to write backup A");
        free(firmware_buffer);
        return;
    }
    
    /* Generate signature */
    uint8_t signature[512];
    size_t sig_size = sizeof(signature);
    if (!crypto_sign(firmware_buffer, FIRMWARE_REGION_SIZE, 
                    signature, &sig_size)) {
        src_log("SRC: ERROR - Failed to generate signature");
        free(firmware_buffer);
        return;
    }
    
    /* Write signature */
    char sig_path[256];
    snprintf(sig_path, sizeof(sig_path), "/SECURITY_RECOVERY/signature.sig");
    src_usb_write_file(sig_path, signature, sig_size);
    
    /* Update manifest and metadata */
    src_update_manifest();
    src_update_metadata(hash);
    
    /* Update config */
    memcpy(config.firmware_hash, hash, 32);
    config.last_backup_timestamp = now;
    src_write_config(&config);
    
    src_log("SRC: Backup completed successfully");
    free(firmware_buffer);
}

/**
 * Check if recovery core is currently disabled
 */
bool src_is_disabled(void) {
    if (!config.enabled) {
        return true;
    }
    
    if (config.disable_until_timestamp > 0) {
        uint32_t now = platform_get_timestamp();
        if (now < config.disable_until_timestamp) {
            return true;
        }
        /* Time expired, clear it */
        config.disable_until_timestamp = 0;
        src_write_config(&config);
    }
    
    return false;
}

/**
 * Disable recovery core temporarily
 */
bool src_disable_temporary(uint32_t duration_ms) {
    if (duration_ms > MAX_DISABLE_DURATION_MS) {
        src_log("SRC: ERROR - Disable duration exceeds maximum");
        return false;
    }
    
    uint32_t now = platform_get_timestamp();
    config.disable_until_timestamp = now + duration_ms;
    src_write_config(&config);
    
    src_log("SRC: Recovery core disabled for %lu ms", duration_ms);
    return true;
}

/**
 * Enable recovery core
 */
bool src_enable(void) {
    config.enabled = true;
    config.disable_until_timestamp = 0;
    src_write_config(&config);
    
    src_log("SRC: Recovery core enabled");
    return true;
}

/**
 * Schedule removal of recovery core
 */
bool src_schedule_removal(void) {
    removal_scheduled = true;
    src_write_config(&config);
    src_log("SRC: Removal scheduled (will complete on next reboot)");
    return true;
}

/**
 * Handle removal process
 */
void src_handle_removal(void) {
    src_log("SRC: Starting removal process");
    
    /* Validate firmware integrity before removal */
    uint8_t *firmware_buffer = malloc(FIRMWARE_REGION_SIZE);
    if (!firmware_buffer) {
        src_log("SRC: ERROR - Memory allocation failed");
        return;
    }
    
    if (!src_read_firmware(firmware_buffer, FIRMWARE_REGION_SIZE, 
                          FIRMWARE_REGION_START)) {
        src_log("SRC: ERROR - Failed to read firmware, aborting removal");
        free(firmware_buffer);
        removal_scheduled = false;
        src_write_config(&config);
        return;
    }
    
    /* Verify firmware hash */
    uint8_t hash[32];
    crypto_sha256(firmware_buffer, FIRMWARE_REGION_SIZE, hash);
    
    if (memcmp(hash, config.firmware_hash, 32) != 0) {
        src_log("SRC: ERROR - Firmware integrity check failed, aborting removal");
        free(firmware_buffer);
        removal_scheduled = false;
        src_write_config(&config);
        return;
    }
    
    /* Restore stock firmware layout */
    /* Clear SRC reserved region */
    uint8_t zero_buffer[4096] = {0};
    for (uint32_t offset = 0; offset < SRC_RESERVED_REGION_SIZE; 
         offset += sizeof(zero_buffer)) {
        spi_flash_write(SRC_RESERVED_REGION_START + offset, 
                       zero_buffer, 
                       sizeof(zero_buffer));
    }
    
    /* Disable recovery logic */
    config.enabled = false;
    src_write_config(&config);
    
    /* Lock SPI flash (if supported) */
    spi_flash_lock();
    
    src_log("SRC: Removal completed successfully");
    free(firmware_buffer);
    
    /* Reboot system */
    system_reboot();
}

/**
 * Verify cryptographic signature
 */
bool src_verify_signature(const uint8_t *firmware, size_t size, 
                         const uint8_t *signature, size_t sig_size) {
    return crypto_verify(firmware, size, signature, sig_size);
}

/**
 * Read configuration from SPI flash
 */
bool src_read_config(src_config_t *config) {
    return spi_flash_read(SRC_RESERVED_REGION_START, 
                         (uint8_t *)config, 
                         sizeof(src_config_t));
}

/**
 * Write configuration to SPI flash
 */
bool src_write_config(const src_config_t *config) {
    return spi_flash_write(SRC_RESERVED_REGION_START, 
                          (const uint8_t *)config, 
                          sizeof(src_config_t));
}

/**
 * Read firmware from SPI flash
 */
bool src_read_firmware(uint8_t *buffer, size_t size, uint32_t offset) {
    return spi_flash_read(offset, buffer, size);
}

/**
 * Write firmware to SPI flash with verification
 */
bool src_write_firmware(const uint8_t *buffer, size_t size, uint32_t offset) {
    /* Write firmware */
    if (!spi_flash_write(offset, buffer, size)) {
        return false;
    }
    
    /* Verify by reading back */
    uint8_t *verify_buffer = malloc(size);
    if (!verify_buffer) {
        return false;
    }
    
    if (!spi_flash_read(offset, verify_buffer, size)) {
        free(verify_buffer);
        return false;
    }
    
    bool match = (memcmp(buffer, verify_buffer, size) == 0);
    free(verify_buffer);
    
    if (!match) {
        src_log("SRC: ERROR - Firmware verification failed after write");
    }
    
    return match;
}
