# Security Recovery Core - Uninstallation Guide

## ⚠️ IMPORTANT WARNINGS

**Before proceeding with uninstallation:**

1. **Your data will remain intact** - SRC never modifies user data partitions
2. **Firmware protection will be permanently lost** - System will be vulnerable to bricking
3. **Process cannot be interrupted** - Interruption may cause firmware corruption
4. **Reboot required** - Removal completes on next system reboot

## Safe Uninstallation Process

### Prerequisites

- Administrator/root access
- System in healthy state (no pending recovery)
- Backup of current firmware (recommended)
- Stable power supply (no interruptions)

### Step 1: Verify System Health

```bash
sudo security status
```

Ensure:
- Status shows "ENABLED" (not in recovery mode)
- Firmware Health is "OK"
- No recent recovery attempts

### Step 2: Backup Current Firmware (Recommended)

Even though SRC maintains backups, create an additional backup:

```bash
# Platform-specific backup command
# Example for Linux:
sudo dd if=/dev/mtd0 of=~/firmware_backup.bin bs=1M
```

### Step 3: Initiate Removal

```bash
sudo security remove
```

**OR with fewer prompts (still requires password):**

```bash
sudo security remove --force
```

### Step 4: Follow Interactive Prompts

The removal process requires **4 separate confirmations**:

1. **OS Password Authentication**
   - Enter your operating system password
   - This prevents unauthorized removal

2. **Confirmation Question 1**
   ```
   Do you understand that firmware protection will be lost? [yes/no]
   ```
   - Type: `yes`

3. **Confirmation Question 2**
   ```
   Have you backed up your firmware? [yes/no]
   ```
   - Type: `yes` (recommended to have backup)

4. **Confirmation Question 3**
   ```
   Type 'REMOVE' to confirm removal:
   ```
   - Type exactly: `REMOVE` (case-sensitive)

5. **Final Warning**
   - 5-second countdown
   - Press Ctrl+C to cancel
   - Otherwise, removal is scheduled

### Step 5: Reboot System

After scheduling removal:

```bash
# Linux/macOS
sudo reboot

# Windows
shutdown /r /t 0
```

**DO NOT INTERRUPT THE REBOOT PROCESS**

### Step 6: Removal Completion

During reboot, Recovery Core will:

1. Validate firmware integrity
2. Verify system is in safe state
3. Clear SRC reserved region in SPI flash
4. Restore stock firmware layout
5. Disable recovery logic
6. Lock SPI flash (if supported)
7. Complete normal boot

**If any validation fails, removal is aborted and system boots normally.**

## Post-Removal Verification

### 1. Check Removal Status

```bash
sudo security status
```

Expected output:
```
Status: NOT INSTALLED
```

### 2. Verify SPI Flash

**Linux:**
```bash
# Check that SRC region is cleared
sudo hexdump -C /dev/mtd0 | grep -A 10 "00100000"
```

**Windows:**
- Use vendor-specific SPI flash tools
- Verify reserved region is empty

### 3. Remove CLI Tool (Optional)

```bash
cd cli
make uninstall
```

## Troubleshooting Removal

### Removal Aborted During Reboot

**Symptoms:**
- System boots normally
- `security status` still shows "ENABLED"
- SRC region not cleared

**Cause:**
- Firmware integrity check failed
- System was in unsafe state
- SPI flash write protection active

**Solution:**
1. Verify system is healthy:
   ```bash
   sudo security status
   ```
2. Ensure no recovery is in progress
3. Retry removal:
   ```bash
   sudo security remove
   ```

### CLI Tool Reports "Removal Scheduled" But Nothing Happens

**Possible Causes:**
- Firmware write failed
- SPI flash locked
- Platform-specific issue

**Solution:**
1. Check firmware logs (platform-specific location)
2. Verify SPI flash is writable
3. Try manual removal (advanced, see below)

### System Won't Boot After Removal

**This should not happen if removal completed successfully.**

**If it does:**
1. Boot from recovery media
2. Restore firmware from backup
3. Check SPI flash for corruption
4. Contact support with logs

## Manual Removal (Advanced)

**⚠️ Only for experts and when automated removal fails**

### Step 1: Disable Recovery Core

```bash
# Temporarily disable (prevents intervention)
sudo security off 1h
```

### Step 2: Access SPI Flash Directly

**Linux:**
```bash
# Backup first!
sudo dd if=/dev/mtd0 of=~/spi_backup.bin

# Clear SRC region (offset 0x100000, size 0x80000)
sudo dd if=/dev/zero of=/dev/mtd0 bs=1 seek=1048576 count=524288
```

**Windows:**
- Use vendor-specific SPI programming tools
- Clear region at offset 0x100000

### Step 3: Verify Removal

```bash
sudo security status
```

## Emergency Recovery

If removal process is interrupted and system is bricked:

1. **Boot from USB Recovery Device**
   - SRC should still be active if removal didn't complete
   - System should auto-recover from USB

2. **Manual Recovery:**
   - Use vendor-specific recovery tools
   - Flash firmware from backup

3. **Contact Support:**
   - Provide removal logs
   - System information
   - Error messages

## Re-Installation After Removal

If you want to reinstall SRC after removal:

1. Follow [INSTALL.md](INSTALL.md) installation guide
2. System will be re-protected
3. Previous backups (if USB still present) may be reused

## Frequently Asked Questions

### Q: Can I cancel removal after scheduling?

**A:** Yes, but only before reboot. After reboot, removal proceeds automatically.

### Q: What happens if I interrupt the reboot?

**A:** Removal may be incomplete. System should boot normally, but SRC may be in inconsistent state. Re-run removal process.

### Q: Will my data be affected?

**A:** No. SRC only modifies firmware regions, never user data partitions.

### Q: Can I remove SRC without rebooting?

**A:** No. Removal requires firmware-level changes that only take effect on reboot.

### Q: What if removal fails validation?

**A:** Removal is aborted, system boots normally, SRC remains active. Fix the issue and retry.

## Support

For removal issues:
- Review logs: `sudo security status`
- Check [TROUBLESHOOTING.md](TROUBLESHOOTING.md)
- Consult platform-specific documentation
- Contact support with error details
