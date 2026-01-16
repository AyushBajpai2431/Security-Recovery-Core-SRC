/**
 * USB Mass Storage Device Implementation
 */

#include "usb_msd.h"
#include "platform.h"
#include <string.h>
#include <stdio.h>

static bool usb_initialized = false;

bool src_usb_init(void) {
    if (usb_initialized) {
        return true;
    }
    
    /* Platform-specific USB initialization */
    if (!platform_usb_init()) {
        return false;
    }
    
    usb_initialized = true;
    return true;
}

bool src_usb_check_present(void) {
    if (!usb_initialized) {
        return false;
    }
    
    /* Check if USB device is connected and mounted */
    return platform_usb_is_present();
}

bool src_usb_read_file(const char *path, uint8_t *buffer, size_t *size) {
    if (!usb_initialized || !path || !buffer || !size) {
        return false;
    }
    
    /* Platform-specific file read */
    return platform_usb_read_file(path, buffer, size);
}

bool src_usb_write_file(const char *path, const uint8_t *buffer, size_t size) {
    if (!usb_initialized || !path || !buffer || size == 0) {
        return false;
    }
    
    /* Platform-specific file write */
    return platform_usb_write_file(path, buffer, size);
}

bool src_usb_delete_file(const char *path) {
    if (!usb_initialized || !path) {
        return false;
    }
    
    /* Platform-specific file delete */
    return platform_usb_delete_file(path);
}

bool src_usb_file_exists(const char *path) {
    if (!usb_initialized || !path) {
        return false;
    }
    
    /* Platform-specific file existence check */
    return platform_usb_file_exists(path);
}

bool src_usb_rename_file(const char *old_path, const char *new_path) {
    if (!usb_initialized || !old_path || !new_path) {
        return false;
    }
    
    /* Platform-specific file rename */
    return platform_usb_rename_file(old_path, new_path);
}

void src_update_manifest(void) {
    char manifest_path[256];
    snprintf(manifest_path, sizeof(manifest_path), 
            "/SECURITY_RECOVERY/manifest.json");
    
    /* Generate manifest JSON */
    char manifest_json[1024];
    snprintf(manifest_json, sizeof(manifest_json),
            "{\n"
            "  \"version\": \"1.0\",\n"
            "  \"board_id\": \"%s\",\n"
            "  \"backup_a\": \"%s\",\n"
            "  \"backup_b\": \"%s\",\n"
            "  \"timestamp\": %lu\n"
            "}\n",
            "DEFAULT",  // Should use actual board_id from config
            "A.bin",
            "B.bin",
            (unsigned long)platform_get_timestamp());
    
    src_usb_write_file(manifest_path, (uint8_t *)manifest_json, 
                      strlen(manifest_json));
}

void src_update_metadata(const uint8_t *firmware_hash) {
    char metadata_path[256];
    snprintf(metadata_path, sizeof(metadata_path), 
            "/SECURITY_RECOVERY/metadata.txt");
    
    /* Generate metadata */
    char metadata[512];
    char hash_str[65];
    for (int i = 0; i < 32; i++) {
        snprintf(&hash_str[i * 2], 3, "%02x", firmware_hash[i]);
    }
    hash_str[64] = '\0';
    
    snprintf(metadata, sizeof(metadata),
            "Firmware Hash: %s\n"
            "Backup Time: %lu\n"
            "SRC Version: %d.%d.%d\n",
            hash_str,
            (unsigned long)platform_get_timestamp(),
            1, 0, 0);
    
    src_usb_write_file(metadata_path, (uint8_t *)metadata, 
                      strlen(metadata));
}
