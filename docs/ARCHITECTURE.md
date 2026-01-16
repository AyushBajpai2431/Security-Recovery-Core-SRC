# Security Recovery Core - Architecture Documentation

## System Overview

Security Recovery Core (SRC) is a hardware-assisted firmware recovery system that operates at the lowest level of the system, before BIOS/UEFI initialization. It provides automatic backup and recovery capabilities to prevent permanent system bricking.

## Architecture Layers

```
┌─────────────────────────────────────┐
│      Operating System / BIOS       │
├─────────────────────────────────────┤
│      Recovery Core Firmware         │  ← SRC runs here
├─────────────────────────────────────┤
│   Platform Hardware (EC/MCU/SPI)   │
└─────────────────────────────────────┘
```

## Component Architecture

### 1. Recovery Core Firmware

**Location:** Reserved SPI flash region (typically 0x100000-0x17FFFF, 512KB)

**Responsibilities:**
- Boot failure detection
- Automatic firmware backup
- USB recovery operations
- State management
- Cryptographic verification

**State Machine:**
```
INIT → CHECKING_BOOT → BOOT_SUCCESS → BACKUP_ACTIVE
                    ↓
              BOOT_FAILED → RECOVERING
```

### 2. Boot Detection System

**Methods:**
1. **GPIO Signal:** Hardware pin set high on successful boot
2. **Watchdog Timer:** Cleared by firmware after successful initialization
3. **POST Codes:** BIOS/UEFI progress codes above threshold (0xA0+)
4. **Firmware Flag:** Set in NVRAM/SPI after successful boot

**Timeout:** 30 seconds (configurable)

**Failure Criteria:** No success signal within timeout period

### 3. USB Recovery System

**Directory Structure:**
```
/SECURITY_RECOVERY/
├── A.bin          # Latest backup (rotated from)
├── B.bin          # Previous backup (rotated from A)
├── manifest.json  # Metadata (board ID, timestamps)
├── signature.sig  # Cryptographic signature
└── metadata.txt   # Human-readable backup info
```

**Backup Rotation:**
1. New backup → `A.bin`
2. Old `A.bin` → `B.bin`
3. Old `B.bin` → Deleted (only 2 backups maintained)

**Recovery Process:**
1. Detect boot failure
2. Initialize USB Mass Storage
3. Read `manifest.json` to determine backup to use
4. Verify cryptographic signature
5. Write firmware to SPI flash
6. Verify write integrity
7. Trigger system reboot

### 4. Cryptographic System

**Algorithms:**
- **Hashing:** SHA-256
- **Signing:** ECDSA-P256 or RSA-2048
- **Key Storage:** Hardware-protected (TPM/secure element)

**Verification Flow:**
1. Read firmware image from USB
2. Calculate SHA-256 hash
3. Read signature from USB
4. Verify signature against public key
5. Proceed only if verification succeeds

### 5. SPI Flash Layout

```
0x000000 - 0x7FFFFF: Main Firmware (8MB)
0x100000 - 0x17FFFF: Recovery Core (512KB)
  ├── 0x100000 - 0x1003FF: Configuration (1KB)
  ├── 0x100400 - 0x17EFFF: Recovery Core Code
  └── 0x17F000 - 0x17FFFF: Logs (4KB)
0x800000 - 0xFFFFFF: Reserved/Other (8MB)
```

### 6. Configuration Management

**Configuration Structure:**
```c
typedef struct {
    bool enabled;                          // Recovery enabled
    uint32_t disable_until_timestamp;      // Temporary disable
    uint32_t last_backup_timestamp;        // Last backup time
    uint32_t last_recovery_timestamp;      // Last recovery time
    char board_id[32];                     // System identifier
    uint8_t firmware_hash[32];             // Current firmware hash
} src_config_t;
```

**Storage:** SPI flash offset 0x100000

**Integrity:** CRC32 + optional cryptographic signature

## Data Flow

### Normal Boot Flow

```
1. Power On
2. Recovery Core Initializes
3. Check Configuration (enabled/disabled)
4. Initialize Boot Detection
5. Start Boot Timeout Timer
6. Monitor Boot Signals
7. Boot Success Detected
8. Perform Backup (if interval elapsed)
9. Continue Normal Boot
```

### Recovery Flow

```
1. Boot Failure Detected (timeout)
2. Initialize USB Mass Storage
3. Verify USB Device Present
4. Read manifest.json
5. Select Backup (A.bin or B.bin)
6. Read Firmware Image
7. Read Signature
8. Verify Cryptographic Signature
9. Write Firmware to SPI Flash
10. Verify Write Integrity
11. Update Configuration
12. Trigger System Reboot
```

### Backup Flow

```
1. System Healthy (boot success)
2. Check Backup Interval (10 minutes)
3. Verify USB Present
4. Read Current Firmware
5. Calculate Hash
6. Compare with Stored Hash
7. If Changed:
   a. Delete Old B.bin
   b. Move A.bin → B.bin
   c. Write New Firmware → A.bin
   d. Generate Signature
   e. Update manifest.json
   f. Update metadata.txt
   g. Update Configuration
```

## Platform Abstraction Layer

### Interface Functions

**SPI Flash:**
- `platform_spi_init()`
- `platform_spi_read(offset, buffer, size)`
- `platform_spi_write(offset, buffer, size)`
- `platform_spi_erase(offset)`
- `platform_spi_lock()`

**USB Mass Storage:**
- `platform_usb_init()`
- `platform_usb_is_present()`
- `platform_usb_read_file(path, buffer, size)`
- `platform_usb_write_file(path, buffer, size)`

**Boot Detection:**
- `platform_boot_detection_init()`
- GPIO, watchdog, POST code monitoring

**Cryptography:**
- `platform_crypto_init()`
- `platform_sha256(data, size, hash)`
- `platform_sign(data, size, signature, sig_size)`
- `platform_verify(data, size, signature, sig_size)`

**System:**
- `platform_get_timestamp()` - Milliseconds since boot
- `system_reboot()` - Trigger system reboot
- `src_enter_safe_mode()` - Enter safe/recovery mode

## CLI Interface

### Communication Method

**Linux:**
- `/dev/mem` direct memory access
- Sysfs interface (`/sys/class/src/`)
- ioctl() system calls

**Windows:**
- WMI (Windows Management Instrumentation)
- Registry (HKLM\SYSTEM\SRC)
- Custom driver interface

**macOS:**
- IOKit framework
- System extensions
- Kernel extensions (deprecated)

### Command Flow

```
User Command → CLI Tool → Platform Interface → Firmware
                ↓
         Configuration File
         (JSON, local storage)
```

## Error Handling

### Failure Modes

1. **SPI Flash Error:**
   - Retry with exponential backoff
   - Log error
   - Enter safe mode if critical

2. **USB Error:**
   - Skip backup (non-critical)
   - Recovery fails if USB not present
   - Log warning

3. **Signature Verification Failure:**
   - Try next backup (B.bin if A.bin fails)
   - Log security event
   - Abort recovery if both fail

4. **Write Verification Failure:**
   - Retry write operation
   - Log critical error
   - Attempt recovery from other backup

### Safe Mode

When critical errors occur:
- Disable automatic recovery
- Log all errors
- Allow manual intervention
- Provide diagnostic information

## Performance Considerations

### Boot Impact

- **Initialization:** < 100ms
- **Boot Detection:** Passive monitoring (no CPU overhead)
- **Backup Operation:** Asynchronous, non-blocking

### Resource Usage

- **SPI Flash:** 512KB reserved region
- **RAM:** < 64KB during operation
- **CPU:** Minimal (mostly idle, periodic checks)

### Backup Performance

- **Backup Frequency:** Every 10 minutes (configurable)
- **Backup Duration:** ~30 seconds for 8MB firmware
- **USB Speed:** Dependent on USB device (USB 2.0 minimum)

## Extensibility

### Adding New Platforms

1. Implement platform abstraction layer
2. Provide SPI flash interface
3. Implement USB Mass Storage support
4. Configure boot detection methods
5. Test on target hardware

### Adding Features

- New boot detection methods
- Additional backup destinations
- Enhanced cryptographic algorithms
- Extended logging capabilities

## Testing Strategy

### Unit Tests

- Individual component testing
- Mock platform interfaces
- Cryptographic function validation

### Integration Tests

- Full recovery flow
- Backup rotation logic
- State machine transitions

### Hardware Tests

- Real hardware platforms
- SPI flash operations
- USB device compatibility
- Boot detection accuracy

## References

- [UEFI Specification](https://uefi.org/specifications)
- [SPI Flash Standards](https://www.jedec.org/)
- [USB Mass Storage Specification](https://www.usb.org/)
- [Embedded Systems Design Patterns](https://en.wikipedia.org/wiki/Embedded_system)
