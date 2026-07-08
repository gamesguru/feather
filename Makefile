BUILD_DIR ?= build
CMAKE ?= cmake
BUILD_TYPE ?= RelWithDebInfo
CMAKE_OPTIONS ?=

MAKEFLAGS_JOBS := $(shell printf '%s\n' "$(MAKEFLAGS)" | sed -n 's/.*-j\([0-9][0-9]*\).*/\1/p' | head -n1)
SYSTEM_JOBS := $(shell getconf _NPROCESSORS_ONLN 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 1)
BUILD_JOBS ?= $(if $(MAKEFLAGS_JOBS),$(MAKEFLAGS_JOBS),$(SYSTEM_JOBS))

default: build

configure:
	@$(CMAKE) -S . -B $(BUILD_DIR) -DCMAKE_BUILD_TYPE=$(BUILD_TYPE) $(CMAKE_OPTIONS)

build: configure
	@$(CMAKE) --build $(BUILD_DIR) --parallel $(BUILD_JOBS)

clean:
	@rm -rf $(BUILD_DIR)

reconfigure: clean build

guix-build:
	@./contrib/guix/guix-build

guix-attest:
	@./contrib/guix/guix-attest

guix-verify:
	@./contrib/guix/guix-verify

guix-clean:
	@./contrib/guix/guix-clean

.PHONY: default configure build clean reconfigure guix-build guix-attest guix-verify guix-clean
