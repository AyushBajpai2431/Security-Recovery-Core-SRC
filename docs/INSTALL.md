# Security Recovery Core - Installation Guide

## Prerequisites

### Hardware Requirements

- System with Embedded Controller (EC) or dedicated MCU
- SPI flash access (typically 16MB)
- USB Mass Storage support
- GPIO or watchdog timer for boot detection

### Software Requirements

- Linux: Root/sudo access, SPI flash driver
- Windows: Administrator privileges, SPI flash driver
- macOS: Root access, IOKit (where supported)

### USB Recovery Device

- USB drive formatted as FAT32
- Minimum 1GB capacity (recommended: 4GB+)
- Contains `/SECURITY_RECOVERY/` directory structure

## USB Device Preparation

### Step 1: Format USB Drive

**Linux/macOS:**
```bash
# Identify USB device (replace /dev/sdX with your device)
sudo fdisk -l

# Format as FAT32
sudo mkfs.vfat -F 32 /dev/sdX1
```

**Windows:**
1. Open Disk Management
2. Right-click USB drive → Format
3. Select FAT32 file system
4. Click Format

### Step 2: Create Directory Structure

```bash
# Mount USB drive (Linux/macOS)
sudo mkdir -p /mnt/usb
sudo mount /dev/sdX1 /mnt/usb

# Create recovery directory
sudo mkdir -p /mnt/usb/SECURITY_RECOVERY

# Windows: Create E:\SECURITY_RECOVERY\ (or appropriate drive letter)
```

### Step 3: Prepare Firmware Backups

Your USB device must contain:

```
/SECURITY_RECOVERY/
├── A.bin          # Latest firmware backup
├── B.bin          # Previous firmware backup (optional for first install)
├── manifest.json  # Metadata (auto-generated)
├── signature.sig  # Cryptographic signature
└── metadata.txt   # Backup information (auto-generated)
```

**Initial Setup:**
- Copy your current firmware image to `A.bin`
- `B.bin` can be empty or a copy of `A.bin` for first install
- Signatures will be generated during installation

## Installation Methods

### Method 1: Interactive Installation (Recommended)

```bash
# Install CLI tool first
cd cli
make install

# Run interactive installer
sudo security install
```

The installer will:
1. Verify USB device presence
2. Check system compatibility
3. Guide you through each step
4. Flash Recovery Core firmware
5. Enable protection

### Method 2: Manual Installation

#### Step 1: Build Firmware

```bash
cd firmware

# For ARM Cortex-M
make PLATFORM=arm

# For RISC-V
make PLATFORM=riscv

# For generic x86/embedded
make PLATFORM=generic
```

#### Step 2: Flash Firmware

**Using OpenOCD (ARM/RISC-V):**
```bash
openocd -f interface.cfg -f target.cfg \
  -c "program build/arm/recovery_core.bin verify reset exit"
```

**Using Platform-Specific Tools:**
- Intel: Flash Programming Tool (FPT)
- AMD: AMD Flash Tool
- Custom: Use vendor-specific flashing utilities

#### Step 3: Install CLI Tool

```bash
cd ../cli
make install
```

#### Step 4: Enable Recovery Core

```bash
sudo security enable
```

## Platform-Specific Instructions

### Linux

1. **Install SPI Flash Driver:**
   ```bash
   sudo modprobe spi-nor
   sudo modprobe mtd
   ```

2. **Grant Access:**
   ```bash
   sudo chmod 666 /dev/mtd0  # Adjust device as needed
   ```

3. **Verify Installation:**
   ```bash
   sudo security status
   ```

### Windows

1. **Install SPI Flash Driver:**
   - Download vendor-specific driver
   - Install via Device Manager

2. **Run as Administrator:**
   - All `security` commands require Administrator privileges
   - Right-click Command Prompt → Run as Administrator

3. **Verify Installation:**
   ```cmd
   security status
   ```

### macOS

1. **Disable System Integrity Protection (SIP) for SPI Access:**
   ```bash
   # Boot into Recovery Mode
   # Open Terminal
   csrutil disable
   # Reboot
   ```

2. **Install IOKit Framework:**
   - Xcode Command Line Tools required

3. **Verify Installation:**
   ```bash
   sudo security status
   ```

## Post-Installation Verification

### 1. Check Status

```bash
sudo security status
```

Expected output:
```
=== Security Recovery Core Status ===

Status: ENABLED
Last Backup: 2024-01-15 10:30:00
USB Device: Present
Firmware Health: OK
Last Recovery: Never
```

### 2. Test Backup System

Wait 10 minutes after successful boot, then check:
- USB device should contain updated `A.bin` and `B.bin`
- `manifest.json` should reflect latest backup
- `metadata.txt` should show backup timestamp

### 3. Test Recovery (Optional - Advanced)

**WARNING: Only test on non-production systems!**

1. Temporarily disable recovery:
   ```bash
   sudo security off 5m
   ```

2. Corrupt firmware (simulate):
   - This should only be done in a controlled test environment

3. Reboot - system should recover automatically

## Troubleshooting

### USB Device Not Detected

- Ensure USB is formatted as FAT32
- Check `/SECURITY_RECOVERY/` directory exists
- Verify USB is mounted correctly
- Try different USB port

### SPI Flash Access Denied

**Linux:**
```bash
sudo chmod 666 /dev/mtd0
# Or add user to appropriate group
sudo usermod -aG dialout $USER
```

**Windows:**
- Run Command Prompt as Administrator
- Check Device Manager for driver issues

### Firmware Build Fails

- Verify toolchain is installed:
  ```bash
  arm-none-eabi-gcc --version  # For ARM
  riscv64-unknown-elf-gcc --version  # For RISC-V
  ```
- Check platform-specific dependencies
- Review compiler error messages

### Recovery Core Not Starting

- Verify firmware was flashed correctly
- Check boot order (Recovery Core must run before BIOS/UEFI)
- Review platform-specific initialization requirements
- Check logs (platform-specific location)

## Uninstallation

See [UNINSTALL.md](UNINSTALL.md) for safe removal instructions.

## Support

For issues and questions:
- Review [SECURITY.md](SECURITY.md) for security considerations
- Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md) for common issues
- Consult platform-specific documentation
