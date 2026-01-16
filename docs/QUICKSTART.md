# Security Recovery Core - Quick Start Guide

## 5-Minute Setup

### Step 1: Prepare USB Device

```bash
# Format USB drive as FAT32
sudo ./tools/create_usb_structure.sh /dev/sdb1

# Copy your firmware to USB
sudo cp your_firmware.bin /mnt/src_usb/SECURITY_RECOVERY/A.bin
```

### Step 2: Build Firmware

```bash
cd firmware
make PLATFORM=arm  # Or your platform
```

### Step 3: Flash Firmware

```bash
# Using OpenOCD or platform-specific tool
make flash
```

### Step 4: Install CLI

```bash
cd ../cli
make install
```

### Step 5: Enable Protection

```bash
sudo security enable
```

### Step 6: Verify

```bash
sudo security status
```

You should see:
```
Status: ENABLED
USB Device: Present
Firmware Health: OK
```

## Common Commands

### Check Status
```bash
sudo security status
```

### Temporarily Disable
```bash
sudo security off 2h  # Disable for 2 hours
```

### Re-enable
```bash
sudo security enable
```

### Interactive Installation
```bash
sudo security install
```

## What Happens Next?

1. **Automatic Backups:** Every 10 minutes, firmware is backed up to USB
2. **Boot Protection:** If boot fails, system auto-recovers from USB
3. **Two Backups:** Always maintains exactly 2 backups (A.bin and B.bin)

## Troubleshooting

- **USB not detected?** Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- **Build fails?** See [BUILD.md](BUILD.md)
- **Need help?** See [INSTALL.md](INSTALL.md) for detailed instructions

## Next Steps

- Read [ARCHITECTURE.md](ARCHITECTURE.md) to understand how it works
- Review [SECURITY.md](SECURITY.md) for security considerations
- Check [UNINSTALL.md](UNINSTALL.md) if you need to remove SRC
