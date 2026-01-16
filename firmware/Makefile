# Security Recovery Core - Firmware Makefile
# Supports multiple platforms: ARM Cortex-M, x86, RISC-V

# Toolchain selection
ifeq ($(PLATFORM),)
PLATFORM := generic
endif

# Compiler flags
CFLAGS := -Wall -Wextra -Werror -Os -ffunction-sections -fdata-sections
CFLAGS += -DSRC_VERSION_MAJOR=1 -DSRC_VERSION_MINOR=0 -DSRC_VERSION_PATCH=0
LDFLAGS := -Wl,--gc-sections

# Source files
SRC_DIR := src
SOURCES := $(SRC_DIR)/main.c
SOURCES += $(SRC_DIR)/recovery_core.c
SOURCES += $(SRC_DIR)/spi_flash.c
SOURCES += $(SRC_DIR)/usb_msd.c
SOURCES += $(SRC_DIR)/boot_detection.c
SOURCES += $(SRC_DIR)/crypto.c
SOURCES += $(SRC_DIR)/logging.c

# Platform-specific sources
PLATFORM_DIR := platform/$(PLATFORM)
ifneq ($(wildcard $(PLATFORM_DIR)),)
SOURCES += $(wildcard $(PLATFORM_DIR)/*.c)
CFLAGS += -I$(PLATFORM_DIR)
endif

# Include directories
INCLUDES := -I$(SRC_DIR) -Iinclude

# Object files
OBJ_DIR := build/$(PLATFORM)
OBJECTS := $(SOURCES:$(SRC_DIR)/%.c=$(OBJ_DIR)/%.o)
OBJECTS += $(SOURCES:$(PLATFORM_DIR)/%.c=$(OBJ_DIR)/%.o)

# Output
TARGET := $(OBJ_DIR)/recovery_core.bin
ELF_TARGET := $(OBJ_DIR)/recovery_core.elf

# Platform-specific toolchain
ifeq ($(PLATFORM),arm)
CC := arm-none-eabi-gcc
OBJCOPY := arm-none-eabi-objcopy
CFLAGS += -mcpu=cortex-m4 -mthumb -mfloat-abi=hard -mfpu=fpv4-sp-d16
else ifeq ($(PLATFORM),riscv)
CC := riscv64-unknown-elf-gcc
OBJCOPY := riscv64-unknown-elf-objcopy
CFLAGS += -march=rv32imac -mabi=ilp32
else
CC := gcc
OBJCOPY := objcopy
CFLAGS += -m32  # 32-bit for embedded systems
endif

.PHONY: all clean flash help

all: $(TARGET)

$(TARGET): $(ELF_TARGET)
	$(OBJCOPY) -O binary $< $@
	@echo "Built: $@"

$(ELF_TARGET): $(OBJECTS)
	$(CC) $(LDFLAGS) -o $@ $^
	@echo "Linked: $@"

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

$(OBJ_DIR)/%.o: $(PLATFORM_DIR)/%.c
	@mkdir -p $(dir $@)
	$(CC) $(CFLAGS) $(INCLUDES) -c $< -o $@

clean:
	rm -rf build/

flash: $(TARGET)
	@echo "Flashing to device..."
	@echo "Platform-specific flash command required"
	@echo "Example: openocd -f interface.cfg -f target.cfg -c 'program $(TARGET) verify reset exit'"

help:
	@echo "Security Recovery Core - Firmware Build System"
	@echo ""
	@echo "Usage: make [PLATFORM=<platform>] [target]"
	@echo ""
	@echo "Platforms:"
	@echo "  PLATFORM=arm     ARM Cortex-M (default: generic)"
	@echo "  PLATFORM=riscv   RISC-V"
	@echo "  PLATFORM=generic Generic x86/embedded"
	@echo ""
	@echo "Targets:"
	@echo "  all      Build firmware binary (default)"
	@echo "  clean    Remove build artifacts"
	@echo "  flash    Flash firmware to device"
	@echo "  help     Show this help message"
