# Security Recovery Core - Troubleshooting Guide

## Common Issues and Solutions

### USB Device Not Detected

**Symptoms:**
- `security status` shows "USB Device: Not Present"
- Backups not being created
- Recovery fails with "USB not found"

**Solutions:**

1. **Check USB Format:**
   ```bash
   # Linux
   sudo fdisk -l /dev/sdX
   # Should show FAT32 filesystem
   ```

2. **Verify Directory Structure:**
   ```bash
   ls -la /mnt/usb/SECURITY_RECOVERY/
   # Should show A.bin, B.bin, manifest.json, etc.
   ```

3. **Check Mount Point:**
   - Linux: `/media/`, `/mnt/`
   - Windows: Check drive letters (E:, F:, etc.)
   - macOS: `/Volumes/`

4. **Try Different USB Port:**
   - Some ports may not be initialized early enough
   - Use USB 2.0 ports (more compatible)

5. **Reformat USB:**
   ```bash
   sudo mkfs.vfat -F 32 /dev/sdX1
   ```

### SPI Flash Access Denied

**Symptoms:**
- "Permission denied" errors
- Cannot enable/disable recovery
- Configuration writes fail

**Solutions:**

**Linux:**
```bash
# Grant access to SPI device
sudo chmod 666 /dev/mtd0

# Or add user to appropriate group
sudo usermod -aG dialout $USER
# Log out and back in

# Check device permissions
ls -l /dev/mtd*
```

**Windows:**
- Run Command Prompt as Administrator
- Check Device Manager for driver issues
- Reinstall SPI flash driver if needed

**macOS:**
- Disable SIP (System Integrity Protection) if required
- Check IOKit permissions
- Verify kernel extension loading

### Firmware Build Fails

**Symptoms:**
- `make` command fails
- Compiler errors
- Linker errors

**Solutions:**

1. **Check Toolchain:**
   ```bash
   arm-none-eabi-gcc --version
   # Should show installed version
   ```

2. **Install Missing Dependencies:**
   ```bash
   # Ubuntu/Debian
   sudo apt-get install gcc-arm-none-eabi

   # Fedora
   sudo dnf install arm-none-eabi-gcc

   # macOS
   brew install arm-none-eabi-gcc
   ```

3. **Check Platform Support:**
   - Verify platform directory exists: `firmware/platform/<PLATFORM>/`
   - Review platform-specific requirements

4. **Clean Build:**
   ```bash
   make clean
   make PLATFORM=arm
   ```

### Recovery Core Not Starting

**Symptoms:**
- System boots normally but SRC doesn't activate
- `security status` shows "NOT INSTALLED"
- No recovery protection active

**Solutions:**

1. **Verify Firmware Flashed:**
   ```bash
   # Check SPI flash contents
   sudo hexdump -C /dev/mtd0 | grep -A 5 "00100000"
   # Should show SRC code, not all zeros
   ```

2. **Check Boot Order:**
   - Recovery Core must run before BIOS/UEFI
   - Verify boot sequence in platform firmware
   - Check if SRC is in boot chain

3. **Review Initialization:**
   - Check platform-specific initialization requirements
   - Verify hardware interfaces (SPI, USB) are accessible
   - Review boot logs

4. **Re-flash Firmware:**
   ```bash
   cd firmware
   make clean
   make PLATFORM=arm
   make flash
   ```

### Boot Detection False Positives

**Symptoms:**
- System boots successfully but recovery triggers
- Frequent unnecessary recovery attempts
- Boot timeout too short

**Solutions:**

1. **Adjust Boot Timeout:**
   - Edit `BOOT_TIMEOUT_MS` in `recovery_core.h`
   - Increase if system takes longer to boot
   - Recompile and flash

2. **Check Boot Detection Methods:**
   - Verify GPIO signal is connected
   - Check watchdog timer configuration
   - Review POST code thresholds

3. **Review Boot Logs:**
   - Check when boot success signals are received
   - Verify timing of signals
   - Adjust detection logic if needed

### Backup Not Creating

**Symptoms:**
- Last backup timestamp not updating
- USB device present but no backups
- Backup interval elapsed but no action

**Solutions:**

1. **Check Backup Interval:**
   ```bash
   security status
   # Check "Last Backup" timestamp
   # Should update every 10 minutes
   ```

2. **Verify USB Write Permissions:**
   ```bash
   # Linux
   ls -ld /mnt/usb/SECURITY_RECOVERY/
   # Should be writable
   ```

3. **Check Firmware Changes:**
   - Backups only occur if firmware hash changes
   - If firmware unchanged, backup is skipped (by design)

4. **Review Logs:**
   - Check for error messages
   - Verify USB device is accessible
   - Check available space on USB

### Signature Verification Fails

**Symptoms:**
- Recovery fails with "signature verification failed"
- Cannot restore from backup
- "Invalid signature" errors

**Solutions:**

1. **Verify Signature File:**
   ```bash
   ls -l /mnt/usb/SECURITY_RECOVERY/signature.sig
   # Should exist and be readable
   ```

2. **Check Cryptographic Keys:**
   - Verify public key matches private key used for signing
   - Check key hasn't been rotated
   - Ensure keys are correct for this system

3. **Re-sign Firmware:**
   ```bash
   # Use signing tool to regenerate signature
   src-sign A.bin -o signature.sig
   ```

4. **Try Other Backup:**
   - If A.bin fails, system should try B.bin automatically
   - Check if B.bin signature is valid

### Removal Process Fails

**Symptoms:**
- `security remove` aborts
- Removal scheduled but doesn't complete on reboot
- Validation errors during removal

**Solutions:**

1. **Check System Health:**
   ```bash
   security status
   # Ensure system is healthy before removal
   ```

2. **Verify Firmware Integrity:**
   - Removal validates firmware before proceeding
   - If validation fails, fix firmware issues first
   - Re-enable recovery and fix problems

3. **Check SPI Flash Lock:**
   - Some systems lock SPI flash
   - May need to unlock before removal
   - Platform-specific procedure

4. **Review Removal Logs:**
   - Check for specific error messages
   - Verify all validation checks pass
   - Retry removal after fixing issues

### Performance Issues

**Symptoms:**
- Slow boot times
- System lag during backup
- High CPU usage

**Solutions:**

1. **Check Backup Frequency:**
   - Default is 10 minutes
   - Reduce frequency if needed (edit source)
   - Backups are asynchronous and shouldn't block

2. **Optimize USB Device:**
   - Use faster USB 3.0 device
   - Ensure USB device is not fragmented
   - Use high-quality USB drive

3. **Review Boot Detection:**
   - Ensure detection methods are efficient
   - Minimize polling frequency
   - Use interrupts where possible

## Diagnostic Commands

### Check System Status

```bash
security status
```

### View Configuration

```bash
# Linux
cat ~/.config/src/config.json

# Windows
type %APPDATA%\SRC\config.json
```

### Check USB Device

```bash
# Linux
ls -la /mnt/usb/SECURITY_RECOVERY/
df -h /mnt/usb

# Windows
dir E:\SECURITY_RECOVERY
```

### Verify SPI Flash

```bash
# Linux
sudo hexdump -C /dev/mtd0 | head -100
```

### Check Logs

```bash
# Platform-specific log location
# Linux: /var/log/src.log or dmesg
# Windows: Event Viewer
# macOS: Console.app
```

## Getting Help

### Information to Provide

When reporting issues, include:

1. **System Information:**
   - OS and version
   - Platform/hardware
   - SRC version

2. **Error Messages:**
   - Exact error text
   - Command that failed
   - Timestamp

3. **Configuration:**
   - `security status` output
   - Relevant config files
   - USB device information

4. **Logs:**
   - System logs
   - SRC logs (if accessible)
   - Boot logs

### Support Channels

- GitHub Issues: [project-url]/issues
- Documentation: [docs-url]
- Community Forum: [forum-url]

## Advanced Troubleshooting

### Manual Recovery

If automatic recovery fails:

1. Boot from USB recovery device manually
2. Use platform-specific recovery tools
3. Flash firmware directly to SPI

### Firmware Corruption

If firmware is corrupted:

1. Boot from recovery media
2. Use backup from USB device
3. Flash using vendor tools
4. Reinstall SRC after recovery

### SPI Flash Corruption

If SPI flash is corrupted:

1. Use external SPI programmer
2. Read backup from USB
3. Flash entire SPI image
4. Verify integrity

## Prevention

### Best Practices

1. **Regular Backups:**
   - Keep USB device connected
   - Verify backups are current
   - Test recovery process periodically

2. **Monitor Status:**
   - Check `security status` regularly
   - Review logs for warnings
   - Address issues promptly

3. **System Maintenance:**
   - Keep firmware updated
   - Maintain USB device health
   - Monitor SPI flash wear

4. **Documentation:**
   - Document system configuration
   - Keep recovery procedures handy
   - Maintain firmware backups
