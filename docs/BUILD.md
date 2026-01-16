# Security Recovery Core - Build Instructions

## Firmware Build

### Prerequisites

**Linux:**
```bash
sudo apt-get install build-essential
sudo apt-get install gcc-arm-none-eabi  # For ARM
sudo apt-get install gcc-riscv64-unknown-elf  # For RISC-V
```

**macOS:**
```bash
brew install arm-none-eabi-gcc
brew install riscv64-unknown-elf-gcc
```

**Windows:**
- Install MSYS2 or WSL
- Install ARM GCC toolchain
- Install Make for Windows

### Building

**Generic Build:**
```bash
cd firmware
make
```

**Platform-Specific Build:**
```bash
# ARM Cortex-M
make PLATFORM=arm

# RISC-V
make PLATFORM=riscv

# Generic x86/embedded
make PLATFORM=generic
```

**Output:**
- Binary: `build/<PLATFORM>/recovery_core.bin`
- ELF: `build/<PLATFORM>/recovery_core.elf`

### Flashing

**Using OpenOCD (ARM/RISC-V):**
```bash
make flash
# Or manually:
openocd -f interface.cfg -f target.cfg \
  -c "program build/arm/recovery_core.bin verify reset exit"
```

**Platform-Specific:**
- Intel: Use Flash Programming Tool (FPT)
- AMD: Use AMD Flash Tool
- Custom: Use vendor-specific tools

## CLI Build

### Prerequisites

- Python 3.6 or higher
- pip (Python package manager)

### Installation

**Linux/macOS:**
```bash
cd cli
make install
```

**Windows:**
```bash
cd cli
# Copy security.py to system PATH
# Or use: python security.py <command>
```

### Development Setup

```bash
cd cli
python3 -m venv venv
source venv/bin/activate  # Linux/macOS
# Or: venv\Scripts\activate  # Windows
pip install -r requirements.txt
```

## Platform-Specific Builds

### ARM Cortex-M

```bash
cd firmware
make PLATFORM=arm CFLAGS="-mcpu=cortex-m4 -mthumb"
```

### RISC-V

```bash
cd firmware
make PLATFORM=riscv CFLAGS="-march=rv32imac"
```

### x86 Embedded

```bash
cd firmware
make PLATFORM=generic CFLAGS="-m32"
```

## Cross-Compilation

### Setting Up Cross-Compiler

**ARM:**
```bash
# Download ARM GCC toolchain
wget https://developer.arm.com/.../gcc-arm-none-eabi-xxx.tar.bz2
tar xjf gcc-arm-none-eabi-xxx.tar.bz2
export PATH=$PATH:$(pwd)/gcc-arm-none-eabi-xxx/bin
```

**RISC-V:**
```bash
# Download RISC-V GCC toolchain
wget https://github.com/riscv/riscv-gnu-toolchain/...
# Build from source or use pre-built
```

### Building for Target

```bash
export CC=arm-none-eabi-gcc
export OBJCOPY=arm-none-eabi-objcopy
make PLATFORM=arm
```

## Debug Builds

### Enable Debug Symbols

```bash
make CFLAGS="-g -O0 -DDEBUG"
```

### Debugging with GDB

```bash
arm-none-eabi-gdb build/arm/recovery_core.elf
(gdb) target remote :3333
(gdb) load
(gdb) continue
```

## Release Builds

### Optimized Build

```bash
make CFLAGS="-Os -flto -DNDEBUG"
```

### Size Optimization

```bash
make CFLAGS="-Os -ffunction-sections -fdata-sections" \
     LDFLAGS="-Wl,--gc-sections"
```

## Testing

### Unit Tests

```bash
cd firmware/tests
make
./run_tests
```

### Integration Tests

```bash
cd tests
python3 -m pytest integration/ -v
```

## Continuous Integration

### GitHub Actions Example

```yaml
name: Build Firmware
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
      - uses: actions/checkout@v2
      - name: Install dependencies
        run: |
          sudo apt-get update
          sudo apt-get install -y gcc-arm-none-eabi
      - name: Build
        run: |
          cd firmware
          make PLATFORM=arm
```

## Build Variants

### Minimal Build (No USB)

```bash
make CFLAGS="-DSRC_NO_USB"
```

### Debug Build

```bash
make CFLAGS="-DDEBUG -g"
```

### Production Build

```bash
make CFLAGS="-Os -DNDEBUG -DSRC_PRODUCTION"
```

## Troubleshooting Build Issues

### Missing Dependencies

```bash
# Check toolchain
arm-none-eabi-gcc --version

# Install missing packages
sudo apt-get install <package-name>
```

### Linker Errors

- Check library paths
- Verify platform-specific libraries
- Review linker script

### Compiler Errors

- Check C standard (C99 required)
- Verify platform defines
- Review include paths

## Build Artifacts

### Generated Files

- `build/<PLATFORM>/recovery_core.bin` - Binary image
- `build/<PLATFORM>/recovery_core.elf` - ELF executable
- `build/<PLATFORM>/*.o` - Object files
- `build/<PLATFORM>/*.d` - Dependency files

### Cleaning

```bash
make clean
# Removes all build artifacts
```
