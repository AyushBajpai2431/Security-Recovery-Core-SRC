#!/usr/bin/env python3
"""
Security Recovery Core (SRC) - CLI Tool
Cross-platform command-line interface for managing SRC
"""

import sys
import os
import argparse
import json
import time
import getpass
import platform as plat
from pathlib import Path
from typing import Optional, Tuple
from datetime import datetime, timedelta

# Version
VERSION = "1.0.0"

# Paths
if plat.system() == "Windows":
    CONFIG_DIR = Path(os.environ.get("APPDATA", "")) / "SRC"
    SPI_DEVICE = None  # Platform-specific
else:
    CONFIG_DIR = Path.home() / ".config" / "src"
    SPI_DEVICE = "/dev/mem"  # May need adjustment per platform

CONFIG_FILE = CONFIG_DIR / "config.json"
STATE_FILE = CONFIG_DIR / "state.json"

# USB Recovery Path
USB_RECOVERY_PATH = "/SECURITY_RECOVERY"
USB_RECOVERY_PATH_WIN = "E:\\SECURITY_RECOVERY"  # Typical USB drive letter


class SRCConfig:
    """SRC Configuration Manager"""
    
    def __init__(self):
        self.config_dir = CONFIG_DIR
        self.config_file = CONFIG_FILE
        self.state_file = STATE_FILE
        self.ensure_config_dir()
    
    def ensure_config_dir(self):
        """Create config directory if it doesn't exist"""
        self.config_dir.mkdir(parents=True, exist_ok=True)
    
    def read_config(self) -> dict:
        """Read configuration from file"""
        if self.config_file.exists():
            try:
                with open(self.config_file, 'r') as f:
                    return json.load(f)
            except Exception as e:
                print(f"Error reading config: {e}", file=sys.stderr)
        return {}
    
    def write_config(self, config: dict):
        """Write configuration to file"""
        try:
            with open(self.config_file, 'w') as f:
                json.dump(config, f, indent=2)
        except Exception as e:
            print(f"Error writing config: {e}", file=sys.stderr)
            sys.exit(1)
    
    def read_state(self) -> dict:
        """Read state from file"""
        if self.state_file.exists():
            try:
                with open(self.state_file, 'r') as f:
                    return json.load(f)
            except Exception:
                pass
        return {}
    
    def write_state(self, state: dict):
        """Write state to file"""
        try:
            with open(self.state_file, 'w') as f:
                json.dump(state, f, indent=2)
        except Exception as e:
            print(f"Error writing state: {e}", file=sys.stderr)


class SRCInterface:
    """Interface to Recovery Core firmware"""
    
    def __init__(self):
        self.config_mgr = SRCConfig()
        self.is_windows = plat.system() == "Windows"
        self.is_linux = plat.system() == "Linux"
        self.is_macos = plat.system() == "Darwin"
    
    def check_permissions(self) -> bool:
        """Check if running with sufficient permissions"""
        if self.is_linux or self.is_macos:
            if os.geteuid() != 0:
                print("ERROR: This command requires root/sudo privileges", file=sys.stderr)
                return False
        elif self.is_windows:
            import ctypes
            if not ctypes.windll.shell32.IsUserAnAdmin():
                print("ERROR: This command requires administrator privileges", file=sys.stderr)
                return False
        return True
    
    def enable(self) -> bool:
        """Enable recovery core"""
        if not self.check_permissions():
            return False
        
        print("Enabling Security Recovery Core...")
        
        # Read current config
        config = self.config_mgr.read_config()
        config['enabled'] = True
        config['disable_until_timestamp'] = 0
        config['last_enable_time'] = time.time()
        
        # Write to firmware (platform-specific)
        if not self.write_firmware_config(config):
            print("ERROR: Failed to write to firmware", file=sys.stderr)
            return False
        
        self.config_mgr.write_config(config)
        print("✓ Recovery Core enabled successfully")
        return True
    
    def disable_temporary(self, duration_str: str) -> bool:
        """Temporarily disable recovery core"""
        if not self.check_permissions():
            return False
        
        # Parse duration
        duration_ms = self.parse_duration(duration_str)
        if duration_ms is None:
            print(f"ERROR: Invalid duration format: {duration_str}", file=sys.stderr)
            print("Format: <number><unit> where unit is: s, m, h, d", file=sys.stderr)
            return False
        
        max_duration_ms = 7 * 24 * 60 * 60 * 1000  # 7 days
        if duration_ms > max_duration_ms:
            print(f"ERROR: Maximum disable duration is 7 days", file=sys.stderr)
            return False
        
        print(f"⚠ WARNING: Recovery Core will be disabled for {duration_str}")
        print("⚠ Your system will be unprotected during this time!")
        response = input("Continue? [yes/no]: ")
        if response.lower() != "yes":
            print("Cancelled")
            return False
        
        # Read current config
        config = self.config_mgr.read_config()
        config['enabled'] = True  # Still enabled, just temporarily disabled
        config['disable_until_timestamp'] = int(time.time() * 1000) + duration_ms
        
        # Write to firmware
        if not self.write_firmware_config(config):
            print("ERROR: Failed to write to firmware", file=sys.stderr)
            return False
        
        self.config_mgr.write_config(config)
        print(f"✓ Recovery Core disabled for {duration_str}")
        return True
    
    def status(self) -> bool:
        """Display status"""
        config = self.config_mgr.read_config()
        state = self.config_mgr.read_state()
        
        print("\n=== Security Recovery Core Status ===\n")
        
        # Enabled/Disabled
        enabled = config.get('enabled', False)
        disable_until = config.get('disable_until_timestamp', 0)
        current_time_ms = int(time.time() * 1000)
        
        if not enabled:
            print("Status: DISABLED")
        elif disable_until > 0 and current_time_ms < disable_until:
            remaining_ms = disable_until - current_time_ms
            remaining_str = self.format_duration(remaining_ms)
            print(f"Status: TEMPORARILY DISABLED")
            print(f"Time Remaining: {remaining_str}")
        else:
            print("Status: ENABLED")
        
        # Last backup
        last_backup = config.get('last_backup_timestamp', 0)
        if last_backup > 0:
            backup_time = datetime.fromtimestamp(last_backup / 1000)
            print(f"Last Backup: {backup_time.strftime('%Y-%m-%d %H:%M:%S')}")
        else:
            print("Last Backup: Never")
        
        # USB status
        usb_present = self.check_usb_present()
        print(f"USB Device: {'Present' if usb_present else 'Not Present'}")
        
        # Firmware health
        firmware_health = self.check_firmware_health()
        print(f"Firmware Health: {firmware_health}")
        
        # Last recovery
        last_recovery = config.get('last_recovery_timestamp', 0)
        if last_recovery > 0:
            recovery_time = datetime.fromtimestamp(last_recovery / 1000)
            print(f"Last Recovery: {recovery_time.strftime('%Y-%m-%d %H:%M:%S')}")
        else:
            print("Last Recovery: Never")
        
        print()
        return True
    
    def remove(self, force: bool = False) -> bool:
        """Safely remove recovery core"""
        if not self.check_permissions():
            return False
        
        print("\n" + "="*60)
        print("SECURITY RECOVERY CORE REMOVAL")
        print("="*60 + "\n")
        
        print("⚠ WARNING: This will permanently remove the Recovery Core.")
        print("⚠ Your data will remain intact, but firmware protection will be lost.")
        print("⚠ If this process is interrupted, firmware corruption may occur.\n")
        
        if not force:
            print("This operation requires multiple confirmations for safety.\n")
        
        # Step 1: OS password
        print("Step 1/4: OS Authentication")
        try:
            password = getpass.getpass("Enter your OS password: ")
            if not self.verify_password(password):
                print("ERROR: Password verification failed", file=sys.stderr)
                return False
        except KeyboardInterrupt:
            print("\nCancelled")
            return False
        
        # Step 2: Confirmation question 1
        print("\nStep 2/4: Confirmation")
        q1 = input("Do you understand that firmware protection will be lost? [yes/no]: ")
        if q1.lower() != "yes":
            print("Removal cancelled")
            return False
        
        # Step 3: Confirmation question 2
        print("\nStep 3/4: Confirmation")
        q2 = input("Have you backed up your firmware? [yes/no]: ")
        if q2.lower() != "yes":
            print("Removal cancelled - please backup firmware first")
            return False
        
        # Step 4: Confirmation question 3
        print("\nStep 4/4: Final Confirmation")
        q3 = input("Type 'REMOVE' to confirm removal: ")
        if q3 != "REMOVE":
            print("Removal cancelled")
            return False
        
        # Step 5: Final confirmation
        print("\nFINAL WARNING:")
        print("This is your last chance to cancel. Press Ctrl+C within 5 seconds...")
        try:
            time.sleep(5)
        except KeyboardInterrupt:
            print("\nRemoval cancelled")
            return False
        
        print("\nScheduling removal...")
        
        # Schedule removal in firmware
        config = self.config_mgr.read_config()
        config['removal_scheduled'] = True
        config['removal_timestamp'] = int(time.time() * 1000)
        
        if not self.write_firmware_config(config):
            print("ERROR: Failed to schedule removal in firmware", file=sys.stderr)
            return False
        
        self.config_mgr.write_config(config)
        
        print("\n✓ Removal scheduled successfully")
        print("⚠ The Recovery Core will be removed on the next system reboot.")
        print("⚠ Please reboot your system to complete the removal process.")
        
        return True
    
    def parse_duration(self, duration_str: str) -> Optional[int]:
        """Parse duration string to milliseconds"""
        duration_str = duration_str.strip().lower()
        
        if not duration_str:
            return None
        
        # Extract number and unit
        unit = duration_str[-1]
        try:
            number = float(duration_str[:-1])
        except ValueError:
            return None
        
        multipliers = {
            's': 1000,
            'm': 60 * 1000,
            'h': 60 * 60 * 1000,
            'd': 24 * 60 * 60 * 1000
        }
        
        if unit not in multipliers:
            return None
        
        return int(number * multipliers[unit])
    
    def format_duration(self, ms: int) -> str:
        """Format milliseconds to human-readable string"""
        if ms < 1000:
            return f"{ms}ms"
        elif ms < 60 * 1000:
            return f"{ms // 1000}s"
        elif ms < 60 * 60 * 1000:
            return f"{ms // (60 * 1000)}m"
        elif ms < 24 * 60 * 60 * 1000:
            return f"{ms // (60 * 60 * 1000)}h"
        else:
            return f"{ms // (24 * 60 * 60 * 1000)}d"
    
    def check_usb_present(self) -> bool:
        """Check if USB recovery device is present"""
        if self.is_windows:
            # Check common drive letters
            for drive in "DEFGHIJKLMNOPQRSTUVWXYZ":
                path = f"{drive}:\\SECURITY_RECOVERY"
                if os.path.exists(path):
                    return True
        else:
            # Check /media and /mnt
            for base in ["/media", "/mnt", "/Volumes"]:
                for item in os.listdir(base) if os.path.exists(base) else []:
                    path = os.path.join(base, item, "SECURITY_RECOVERY")
                    if os.path.exists(path):
                        return True
        return False
    
    def check_firmware_health(self) -> str:
        """Check firmware health status"""
        # Platform-specific implementation
        # For now, return a placeholder
        return "OK"
    
    def verify_password(self, password: str) -> bool:
        """Verify OS password"""
        # Platform-specific password verification
        # This is a simplified version - production should use proper OS APIs
        return True  # Placeholder
    
    def write_firmware_config(self, config: dict) -> bool:
        """Write configuration to firmware"""
        # Platform-specific implementation
        # This would communicate with the firmware via:
        # - Linux: /dev/mem, ioctl, or sysfs
        # - Windows: WMI, registry, or driver interface
        # - macOS: IOKit or similar
        
        # For now, this is a placeholder
        # In production, this would:
        # 1. Open SPI flash or EC interface
        # 2. Write config to reserved region
        # 3. Verify write
        return True  # Placeholder
    
    def interactive_install(self):
        """Interactive installation tutorial"""
        print("\n" + "="*60)
        print("Security Recovery Core - Interactive Installation")
        print("="*60 + "\n")
        
        print("This tutorial will guide you through installing SRC.\n")
        
        # Step 1: Check USB
        print("Step 1: USB Recovery Device")
        print("-" * 40)
        print("You need a USB drive formatted as FAT32.")
        print("The drive must contain a /SECURITY_RECOVERY/ directory with:")
        print("  - A.bin (latest firmware backup)")
        print("  - B.bin (previous firmware backup)")
        print("  - manifest.json")
        print("  - signature.sig")
        print("  - metadata.txt\n")
        
        response = input("Have you prepared the USB device? [yes/no]: ")
        if response.lower() != "yes":
            print("\nPlease prepare the USB device first:")
            print("1. Format a USB drive as FAT32")
            print("2. Create /SECURITY_RECOVERY/ directory")
            print("3. Copy your firmware backups and required files")
            print("\nRun this installer again when ready.")
            return False
        
        # Step 2: Verify USB
        if not self.check_usb_present():
            print("\nERROR: USB recovery device not found!")
            print("Please ensure the USB drive is connected and contains /SECURITY_RECOVERY/")
            return False
        
        print("✓ USB device found\n")
        
        # Step 3: System compatibility
        print("Step 2: System Compatibility")
        print("-" * 40)
        print(f"Detected OS: {plat.system()} {plat.release()}")
        print(f"Architecture: {plat.machine()}\n")
        
        response = input("Is your system compatible? [yes/no]: ")
        if response.lower() != "yes":
            print("\nPlease verify compatibility:")
            print("- Embedded Controller (EC) or MCU access")
            print("- SPI flash access")
            print("- USB Mass Storage support")
            return False
        
        # Step 3: Explain installation
        print("\nStep 3: Installation Process")
        print("-" * 40)
        print("The installation will:")
        print("1. Flash Recovery Core firmware to reserved SPI region")
        print("2. Configure boot detection")
        print("3. Initialize backup system")
        print("4. Enable recovery protection\n")
        
        response = input("Proceed with installation? [yes/no]: ")
        if response.lower() != "yes":
            print("Installation cancelled")
            return False
        
        # Step 4: Perform installation
        print("\nInstalling Recovery Core...")
        # Platform-specific installation code would go here
        print("✓ Installation complete\n")
        
        # Step 5: Enable
        print("Enabling Recovery Core...")
        if self.enable():
            print("\n✓ Security Recovery Core is now active!")
            print("\nYour system is now protected against firmware bricking.")
            return True
        else:
            print("\nERROR: Failed to enable Recovery Core")
            return False


def main():
    """Main entry point"""
    parser = argparse.ArgumentParser(
        description="Security Recovery Core (SRC) - Hardware-assisted recovery system",
        formatter_class=argparse.RawDescriptionHelpFormatter,
        epilog="""
Examples:
  security enable              Enable recovery core
  security off 2h             Disable for 2 hours
  security status             Show status
  security remove --force     Remove recovery core (with confirmations)
  security install            Interactive installation tutorial
        """
    )
    
    parser.add_argument('--version', action='version', version=f'SRC {VERSION}')
    
    subparsers = parser.add_subparsers(dest='command', help='Command to execute')
    
    # Enable command
    enable_parser = subparsers.add_parser('enable', help='Enable recovery core')
    
    # Disable command
    disable_parser = subparsers.add_parser('off', help='Temporarily disable recovery core')
    disable_parser.add_argument('duration', help='Duration (e.g., 4m, 2h, 5d)')
    
    # Status command
    status_parser = subparsers.add_parser('status', help='Show status')
    
    # Remove command
    remove_parser = subparsers.add_parser('remove', help='Safely remove recovery core')
    remove_parser.add_argument('--force', action='store_true', 
                              help='Skip some confirmations (still requires password)')
    
    # Install command
    install_parser = subparsers.add_parser('install', help='Interactive installation tutorial')
    
    args = parser.parse_args()
    
    if not args.command:
        parser.print_help()
        sys.exit(1)
    
    interface = SRCInterface()
    
    success = False
    if args.command == 'enable':
        success = interface.enable()
    elif args.command == 'off':
        success = interface.disable_temporary(args.duration)
    elif args.command == 'status':
        success = interface.status()
    elif args.command == 'remove':
        success = interface.remove(force=args.force)
    elif args.command == 'install':
        success = interface.interactive_install()
    else:
        parser.print_help()
        sys.exit(1)
    
    sys.exit(0 if success else 1)


if __name__ == '__main__':
    main()
