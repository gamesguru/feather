#!/bin/bash

# Configure CMake with custom flags
# - FEATHER_VERSION_DEBUG_BUILD=ON : Use git describe for version string
# - CHECK_UPDATES=OFF              : Disable built-in update checker
# - USE_DEVICE_TREZOR=OFF          : Disable Trezor hardware wallet support
# - WITH_SCANNER=OFF               : Disable webcam QR scanner support

cmake \
    -DCMAKE_BUILD_TYPE=Debug \
    -DFEATHER_VERSION_DEBUG_BUILD=ON \
    -DCHECK_UPDATES=OFF \
    -DUSE_DEVICE_TREZOR=OFF \
    -DWITH_SCANNER=OFF \
    ..
