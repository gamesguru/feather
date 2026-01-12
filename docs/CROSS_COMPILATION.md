# Cross-Compilation Guide

This document outlines how to cross-compile Feather Wallet for **Windows** and **macOS** from a Linux host.

## 1. Official Release Builds (Guix)

Feather uses **[Guix](https://guix.gnu.org/)** for reproducible release builds. This is the **recommended** method for producing binaries that match official releases.

### Requirements
*   A Linux distribution (Debian/Ubuntu recommended)
*   ~50GB free disk space
*   [Guix installed](https://guix.gnu.org/manual/en/html_node/Binary-Installation.html)

### Windows Build
From the repository root:
```bash
HOSTS="x86_64-w64-mingw32" make build
```
*   **Result**: `feather.exe` (and installer if specified) will be in `contrib/guix/guix-build-<hash>/output/`.

### macOS Build
From the repository root:
```bash
HOSTS="x86_64-apple-darwin arm64-apple-darwin" make build
```
*   **Result**: `.dmg` or `.app` bundles for Intel and Apple Silicon.

### Notes
*   The first run will take a significant amount of time to download and bootstrap the toolchain.
*   See `contrib/guix/README.md` for advanced configuration (`JOBS`, `SUBSTITUTE_URLS`, etc.).

---

## 2. Development Builds (Depends)

For faster, non-reproducible development builds, you can use the `contrib/depends` system.

### Prerequisites
*   `build-essential`, `cmake`, `git`
*   **Windows**: `g++-mingw-w64-x86-64` (Debian/Ubuntu) or equivalent.
*   **macOS**: `clang`, `libtapi-dev` (or a mac SDK).

### Windows (MinGW)
1.  **Build Dependencies**:
    ```bash
    make -C contrib/depends HOST=x86_64-w64-mingw32
    ```
2.  **Configure and Build**:
    ```bash
    mkdir build-win
    cd build-win
    cmake -DCMAKE_TOOLCHAIN_FILE=../contrib/depends/x86_64-w64-mingw32/share/toolchain.cmake ..
    make
    ```

### macOS
1.  **Get SDK**: You may need to provide a macOS SDK tarball in `contrib/depends/SDKs`.
2.  **Build Dependencies**:
    ```bash
    make -C contrib/depends HOST=x86_64-apple-darwin
    ```
3.  **Configure and Build**:
    ```bash
    mkdir build-mac
    cd build-mac
    cmake -DCMAKE_TOOLCHAIN_FILE=../contrib/depends/x86_64-apple-darwin/share/toolchain.cmake ..
    make
    ```
