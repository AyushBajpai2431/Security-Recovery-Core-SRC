/**
 * USB Mass Storage Device Interface
 * Provides access to USB storage devices (FAT32 only)
 */

#ifndef USB_MSD_H
#define USB_MSD_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/* Initialize USB Mass Storage interface */
bool src_usb_init(void);

/* Check if USB device is present and valid */
bool src_usb_check_present(void);

/* Read file from USB device */
bool src_usb_read_file(const char *path, uint8_t *buffer, size_t *size);

/* Write file to USB device */
bool src_usb_write_file(const char *path, const uint8_t *buffer, size_t size);

/* Delete file from USB device */
bool src_usb_delete_file(const char *path);

/* Check if file exists */
bool src_usb_file_exists(const char *path);

/* Rename file */
bool src_usb_rename_file(const char *old_path, const char *new_path);

/* Update manifest.json */
void src_update_manifest(void);

/* Update metadata.txt */
void src_update_metadata(const uint8_t *firmware_hash);

#endif /* USB_MSD_H */
