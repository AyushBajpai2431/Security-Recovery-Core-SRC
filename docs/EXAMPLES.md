# Security Recovery Core - Examples and Use Cases

## Example Terminal Outputs

### Status Command

```bash
$ sudo security status

=== Security Recovery Core Status ===

Status: ENABLED
Last Backup: 2024-01-15 14:30:22
USB Device: Present
Firmware Health: OK
Last Recovery: Never
```

### Enable Command

```bash
$ sudo security enable
Enabling Security Recovery Core...
✓ Recovery Core enabled successfully
```

### Temporary Disable

```bash
$ sudo security off 2h
⚠ WARNING: Recovery Core will be disabled for 2h
⚠ Your system will be unprotected during this time!
Continue? [yes/no]: yes
✓ Recovery Core disabled for 2h
```

### Status After Temporary Disable

```bash
$ sudo security status

=== Security Recovery Core Status ===

Status: TEMPORARILY DISABLED
Time Remaining: 1h 45m
Last Backup: 2024-01-15 14:30:22
USB Device: Present
Firmware Health: OK
Last Recovery: Never
```

### Removal Process

```bash
$ sudo security remove

============================================================
SECURITY RECOVERY CORE REMOVAL
============================================================

⚠ WARNING: This will permanently remove the Recovery Core.
⚠ Your data will remain intact, but firmware protection will be lost.
⚠ If this process is interrupted, firmware corruption may occur.

This operation requires multiple confirmations for safety.

Step 1/4: OS Authentication
Enter your OS password: ********

Step 2/4: Confirmation
Do you understand that firmware protection will be lost? [yes/no]: yes

Step 3/4: Confirmation
Have you backed up your firmware? [yes/no]: yes

Step 4/4: Final Confirmation
Type 'REMOVE' to confirm removal: REMOVE

FINAL WARNING:
This is your last chance to cancel. Press Ctrl+C within 5 seconds...
(5 second countdown)

Scheduling removal...
✓ Removal scheduled successfully
⚠ The Recovery Core will be removed on the next system reboot.
⚠ Please reboot your system to complete the removal process.
```

## Use Case Scenarios

### Scenario 1: Normal Operation

**Situation:** System running normally with SRC enabled

**What Happens:**
1. System boots successfully
2. Recovery Core detects boot success via GPIO/watchdog
3. Every 10 minutes, firmware is backed up to USB
4. Backups rotate: new → A.bin, old A → B.bin, old B deleted
5. System continues normal operation

**User Action:** None required (automatic)

### Scenario 2: Firmware Update Failure

**Situation:** Firmware update fails, system won't boot

**What Happens:**
1. System attempts to boot
2. Boot fails (no success signal within 30 seconds)
3. Recovery Core detects boot failure
4. USB device checked for presence
5. Recovery Core reads A.bin from USB
6. Signature verified
7. Firmware written to SPI flash
8. System reboots
9. System boots successfully from recovered firmware

**User Action:** None required (automatic recovery)

### Scenario 3: Temporary Maintenance

**Situation:** User needs to disable recovery temporarily for testing

**User Action:**
```bash
sudo security off 30m
# Perform testing
# Recovery automatically re-enables after 30 minutes
```

**What Happens:**
1. Recovery Core disabled for 30 minutes
2. System operates without recovery protection
3. After 30 minutes, recovery automatically re-enables
4. User receives warning when disabling

### Scenario 4: USB Device Removed

**Situation:** USB recovery device is unplugged

**What Happens:**
1. Backup operations skip (USB not present)
2. Recovery Core logs warning
3. System continues normal operation
4. If boot failure occurs, recovery cannot proceed (USB required)
5. User should reconnect USB device

**User Action:**
- Reconnect USB device
- Check `security status` to verify USB detected

### Scenario 5: Both Backups Corrupted

**Situation:** Both A.bin and B.bin have invalid signatures

**What Happens:**
1. Boot failure detected
2. Recovery attempts A.bin → signature fails
3. Recovery attempts B.bin → signature fails
4. Recovery aborts
5. System enters safe mode
6. Manual intervention required

**User Action:**
- Provide valid firmware backup on USB
- Or use platform-specific recovery tools

## Example USB Directory Structure

```
/SECURITY_RECOVERY/
├── A.bin              (8,388,608 bytes - current firmware)
├── B.bin              (8,388,608 bytes - previous firmware)
├── manifest.json      (256 bytes)
├── signature.sig      (512 bytes - ECDSA signature)
└── metadata.txt       (128 bytes)
```

### Example manifest.json

```json
{
  "version": "1.0",
  "board_id": "BOARD-12345",
  "backup_a": "A.bin",
  "backup_b": "B.bin",
  "timestamp": 1705327822000
}
```

### Example metadata.txt

```
Firmware Hash: a3f5c8e9d2b1a4f6c7e8d9b0a1f2c3d4e5f6a7b8c9d0e1f2a3b4c5d6e7f8a9b0
Backup Time: 2024-01-15 14:30:22
SRC Version: 1.0.0
Board ID: BOARD-12345
```

## Example Platform Implementation

See `firmware/platform/generic/platform.c` for a complete example of platform-specific functions.

## Example Build Output

```bash
$ cd firmware
$ make PLATFORM=arm

arm-none-eabi-gcc -Wall -Wextra -Werror -Os ... -c src/main.c -o build/arm/main.o
arm-none-eabi-gcc -Wall -Wextra -Werror -Os ... -c src/recovery_core.c -o build/arm/recovery_core.o
...
arm-none-eabi-gcc -Wl,--gc-sections -o build/arm/recovery_core.elf build/arm/*.o
arm-none-eabi-objcopy -O binary build/arm/recovery_core.elf build/arm/recovery_core.bin
Built: build/arm/recovery_core.bin

$ ls -lh build/arm/
total 512K
-rw-r--r-- 1 user user 256K recovery_core.bin
-rw-r--r-- 1 user user 512K recovery_core.elf
```

## Example Installation Flow

```bash
$ sudo security install

============================================================
Security Recovery Core - Interactive Installation
============================================================

This tutorial will guide you through installing SRC.

Step 1: USB Recovery Device
----------------------------------------
You need a USB drive formatted as FAT32.
The drive must contain a /SECURITY_RECOVERY/ directory with:
  - A.bin (latest firmware backup)
  - B.bin (previous firmware backup)
  - manifest.json
  - signature.sig
  - metadata.txt

Have you prepared the USB device? [yes/no]: yes
✓ USB device found

Step 2: System Compatibility
----------------------------------------
Detected OS: Linux 6.18.4
Architecture: x86_64

Is your system compatible? [yes/no]: yes

Step 3: Installation Process
----------------------------------------
The installation will:
1. Flash Recovery Core firmware to reserved SPI region
2. Configure boot detection
3. Initialize backup system
4. Enable recovery protection

Proceed with installation? [yes/no]: yes

Installing Recovery Core...
✓ Installation complete

Enabling Recovery Core...
✓ Recovery Core enabled successfully

✓ Security Recovery Core is now active!

Your system is now protected against firmware bricking.
```

## Example Recovery Log

```
[2024-01-15 14:30:00] SRC: Initializing Recovery Core v1.0.0
[2024-01-15 14:30:00] SRC: SPI flash initialization complete
[2024-01-15 14:30:00] SRC: USB initialization complete
[2024-01-15 14:30:00] SRC: Initialization complete, monitoring boot
[2024-01-15 14:30:05] SRC: Boot success - GPIO signal received
[2024-01-15 14:30:05] SRC: Starting automatic backup
[2024-01-15 14:30:35] SRC: Backup completed successfully
[2024-01-15 14:40:35] SRC: Starting automatic backup
[2024-01-15 14:40:45] SRC: Firmware unchanged, skipping backup
```
