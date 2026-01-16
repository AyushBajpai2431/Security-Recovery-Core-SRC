#!/bin/bash
#
# Create USB Recovery Device Structure
# Prepares a USB drive with the required directory structure for SRC
#

set -e

USB_DEVICE="${1:-}"
RECOVERY_DIR="SECURITY_RECOVERY"

if [ -z "$USB_DEVICE" ]; then
    echo "Usage: $0 <usb_device>"
    echo "Example: $0 /dev/sdb1"
    echo ""
    echo "Available USB devices:"
    lsblk -o NAME,SIZE,TYPE,MOUNTPOINT | grep -E "sd[a-z][0-9]|disk"
    exit 1
fi

# Check if running as root
if [ "$EUID" -ne 0 ]; then
    echo "ERROR: This script must be run as root (use sudo)"
    exit 1
fi

# Check if device exists
if [ ! -e "$USB_DEVICE" ]; then
    echo "ERROR: Device $USB_DEVICE does not exist"
    exit 1
fi

# Confirm before proceeding
echo "WARNING: This will format $USB_DEVICE as FAT32"
echo "All data on this device will be lost!"
read -p "Continue? [yes/no]: " confirm
if [ "$confirm" != "yes" ]; then
    echo "Cancelled"
    exit 0
fi

# Unmount if mounted
if mountpoint -q "$USB_DEVICE" 2>/dev/null || grep -q "$USB_DEVICE" /proc/mounts; then
    echo "Unmounting $USB_DEVICE..."
    umount "$USB_DEVICE" || true
fi

# Format as FAT32
echo "Formatting $USB_DEVICE as FAT32..."
mkfs.vfat -F 32 "$USB_DEVICE"

# Create mount point
MOUNT_POINT="/mnt/src_usb"
mkdir -p "$MOUNT_POINT"

# Mount USB device
echo "Mounting $USB_DEVICE..."
mount "$USB_DEVICE" "$MOUNT_POINT"

# Create directory structure
echo "Creating directory structure..."
mkdir -p "$MOUNT_POINT/$RECOVERY_DIR"

# Create placeholder files
echo "Creating placeholder files..."

# Create manifest.json
cat > "$MOUNT_POINT/$RECOVERY_DIR/manifest.json" <<EOF
{
  "version": "1.0",
  "board_id": "DEFAULT",
  "backup_a": "A.bin",
  "backup_b": "B.bin",
  "timestamp": 0
}
EOF

# Create metadata.txt
cat > "$MOUNT_POINT/$RECOVERY_DIR/metadata.txt" <<EOF
Firmware Hash: (to be filled)
Backup Time: (to be filled)
SRC Version: 1.0.0
EOF

# Create empty backup files (user must add firmware)
touch "$MOUNT_POINT/$RECOVERY_DIR/A.bin"
touch "$MOUNT_POINT/$RECOVERY_DIR/B.bin"
touch "$MOUNT_POINT/$RECOVERY_DIR/signature.sig"

echo ""
echo "✓ USB device structure created successfully!"
echo ""
echo "Next steps:"
echo "1. Copy your firmware image to: $MOUNT_POINT/$RECOVERY_DIR/A.bin"
echo "2. Generate signature for A.bin"
echo "3. Update manifest.json with your board_id"
echo ""
echo "Device mounted at: $MOUNT_POINT"
echo "Unmount when done: umount $MOUNT_POINT"
