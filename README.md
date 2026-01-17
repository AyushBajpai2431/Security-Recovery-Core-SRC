# Security Recovery Core (SRC)

**Official Website:** [https://security-recovery-core.droploot.org/](https://security-recovery-core.droploot.org/)

**Wiki** [Security Recovery Core Wiki](https://security-recovery-core.droploot.org/wiki.html)

**Hardware-Assisted, Unbrickable, Pre-OS Firmware Recovery**

## Overview

Security Recovery Core (SRC) is an advanced, hardware-assisted system engineered to deliver fail-safe firmware recovery and tamper-resistant protection for a wide variety of computing devices and architectures. The system operates at the lowest system level—well before BIOS/UEFI initialization—detecting, blocking, and repairing firmware corruption, accidental flashing errors, or firmware-targeted attacks. All recovery actions are performed with full preservation of user data, operating system, and system configuration.

SRC ensures that your system is truly "unbrickable," always capable of restoring a trusted firmware state, reducing critical downtime and eliminating the risk of device loss—even in catastrophic scenarios.

## Supported Devices & Hardware Specifications

SRC is compatible with a broad, growing range of devices and configurations, including:

- **CPU Architectures**:  
  - x86 (Intel 6th Gen [Skylake] and newer, AMD Zen/Zen2/Zen3)  
  - ARMv7, ARMv8/v9 (including Raspberry Pi, NVIDIA Jetson, and select Qualcomm Snapdragon boards)  
  - RISC-V (SiFive boards, Allwinner D1 series—beta support)
- **Motherboards**:  
  - Mainstream ATX, Mini-ITX, and embedded industrial boards from vendors like ASUS, ASRock, Supermicro, Gigabyte  
  - Popular development platforms (Raspberry Pi, BeagleBone, Pine64, NVIDIA Jetson Nano/Orin, ODROID, Orange Pi, etc.)
- **Embedded Controllers/MCUs**:  
  - Nuvoton, Renesas, Microchip/Atmel, STM32, Espressif ESP32 (for SPI recovery host)
- **Dedicated Storage Integration**:  
  - SPI NOR/NAND flash (Winbond, Macronix, Micron, etc.), eMMC recovery boot
- **RAM**:  
  - Devices with 512MB RAM or more; optimal operation with ≥1GB RAM on host system
- **GPU**:  
  - Compatible with integrated (Intel/AMD/ARM Mali) and discrete (NVIDIA, AMD) GPUs; SRC operates independently of GPU/graphics stack for maximum compatibility
- **Peripheral Support**:  
  - USB host support for recovery key, SD card, or external drive recovery  
  - GPIO and UART for board-level debug and recovery triggers

Please see [INSTALL.md](docs/INSTALL.md) and the website's hardware matrix for continuously updated device support lists.

## Key Features

- **Truly Unbrickable**: SRC operates outside the system firmware on a dedicated embedded microcontroller (EC/MCU) or isolated SPI flash, remaining fully independent from the host OS, BIOS, or UEFI.
- **Automated Recovery**: Actively detects failed boots, firmware corruption, and allows for recovery triggers—no manual intervention required.
- **Structured Backup Rotation**: Keeps two cryptographically-signed backup firmware copies, allowing instant rollback in case of upgrade, corruption, or security incident.
- **Data Preservation**: Targets platform firmware exclusively; user data, the operating system, and configuration files remain untouched and safe.
- **Tamper-Proof Recovery**: All actions are digitally signed, logged, and verified for cryptographic integrity and auditability.

## Architecture

- **Recovery Core Firmware**: Runs on an EC/MCU or reserved flash (as an isolated recovery root), pre-empting BIOS/UEFI even in failure conditions.
- **Boot Failure Detection**: Multi-layered—hardware (watchdog timers, GPIO/strap, POST code analysis), plus diagnostics via external host CLI.
- **USB & SD-Based Recovery**: Insert a prepared recovery USB stick or SD card for automatic restore, with fallback to signed onboard backups.
- **Cross-Platform CLI**: Secure command-line utilities for Linux, Windows, and macOS facilitate status checks, backup/restore, and system audit.
- **APIs for Integrators**: Clean hooks and APIs for board manufacturers and integrators to incorporate custom recovery or threat-detection routines.

## Quick Start

1. **Access the official website for guides and downloads:**  
   [https://security-recovery-core.droploot.org/](https://security-recovery-core.droploot.org/)
2. **Read the hardware installation documentation:**  
   Step-by-step walkthroughs for supported devices are found in [INSTALL.md](docs/INSTALL.md).
3. **Install the host-based CLI and tools:**  
   See the setup instructions on the official website or in the `cli/` folder.
4. **Simulate and verify recovery:**  
   Use the included test suites to simulate firmware failure conditions and confirm restoration.

## Project Structure

```
SRC/
├── firmware/          # Core recovery firmware and board-specific builds
├── cli/              # Desktop/host command-line interface tools
├── docs/             # Documentation, hardware integration guides, API references
├── tools/            # Build scripts, flashing and emulation utilities
└── tests/            # Automated and hardware-in-the-loop test suites
```

## Security Model

- **Strong Signing**: All firmware and recovery images are secured with elliptic-curve or RSA digital signatures.
- **Tamper-Resistant Logging**: Every recovery operation is time-stamped and cryptographically verified to guarantee auditability.
- **Multi-step Safeguards**: Robust, multi-factor uninstall/disable handles block accidental or unauthorized removal.
- **Hardware Enforced Protections**: Employs SPI flash locking, firmware-level protections, and strict EC/MCU isolation.
- **Open Source, Open Audits**: Codebase, security build process, and audit logs are public and community-auditable.

## License

SRC is released as open-source software (see LICENSE for details), and the project invites contributions, porting to new boards, and independent audits from the wider security and hardware community.

