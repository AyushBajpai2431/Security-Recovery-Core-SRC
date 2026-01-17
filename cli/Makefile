# Security Recovery Core - CLI Makefile
# Cross-platform build system for CLI tool

PYTHON := python3
PIP := pip3
INSTALL_DIR := /usr/local/bin
INSTALL_DIR_WIN := C:\Program Files\SRC\bin

.PHONY: all install uninstall test clean help

all:
	@echo "CLI tool is Python-based, no compilation needed"
	@echo "Run 'make install' to install"

install:
	@echo "Installing Security Recovery Core CLI..."
	@if [ "$(shell uname -s)" = "Linux" ] || [ "$(shell uname -s)" = "Darwin" ]; then \
		sudo cp src/security.py $(INSTALL_DIR)/security; \
		sudo chmod +x $(INSTALL_DIR)/security; \
		echo "Installed to $(INSTALL_DIR)/security"; \
	elif [ "$(shell uname -o 2>/dev/null)" = "Msys" ] || [ -n "$$WINDIR" ]; then \
		mkdir -p "$(INSTALL_DIR_WIN)"; \
		cp src/security.py "$(INSTALL_DIR_WIN)/security.py"; \
		echo "Installed to $(INSTALL_DIR_WIN)/security.py"; \
		echo "Add $(INSTALL_DIR_WIN) to PATH"; \
	fi

uninstall:
	@if [ "$(shell uname -s)" = "Linux" ] || [ "$(shell uname -s)" = "Darwin" ]; then \
		sudo rm -f $(INSTALL_DIR)/security; \
		echo "Uninstalled from $(INSTALL_DIR)/security"; \
	elif [ -n "$$WINDIR" ]; then \
		rm -f "$(INSTALL_DIR_WIN)/security.py"; \
		echo "Uninstalled from $(INSTALL_DIR_WIN)/security.py"; \
	fi

test:
	$(PYTHON) -m pytest tests/ -v

clean:
	find . -type f -name "*.pyc" -delete
	find . -type d -name "__pycache__" -delete
	find . -type d -name "*.egg-info" -exec rm -rf {} +

help:
	@echo "Security Recovery Core - CLI Build System"
	@echo ""
	@echo "Targets:"
	@echo "  all       Show info (no compilation needed)"
	@echo "  install   Install CLI tool to system"
	@echo "  uninstall Remove CLI tool from system"
	@echo "  test      Run tests"
	@echo "  clean     Remove Python cache files"
	@echo "  help      Show this help message"
