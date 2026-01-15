Yes, you absolutely can (and should) verify this locally to save time on CI runs.

Since you are running Arch Linux (Rolling Release / Qt 6) but need to verify a build for Ubuntu 22.04 (Qt 5.15), you typically don't "cross-compile" in the traditional sense (which usually means building for a different CPU architecture like ARM). Instead, you **containerize**.

The best way to verify your fixes is to run a **Docker** container that mirrors the Ubuntu 22.04 CI environment exactly.

### **Option 1: Using Docker (Recommended)**

This allows you to compile against the exact Qt 5 versions used in Ubuntu 22.04 without messing up your host Arch system.

**1. Start an Ubuntu 22.04 container mounting your current source code:**
Run this from your feather repository root:

```bash
# -v mounts your current directory to /feather inside the container
docker run -it --rm -v $(pwd):/feather ubuntu:22.04 bash

```

**2. Inside the container, install the dependencies:**
(I copied this list directly from your `.github/workflows/build.yml` file for the Ubuntu 22 job)

```bash
apt update
apt -y install git cmake build-essential ccache libssl-dev libunbound-dev \
libboost-all-dev libqrencode-dev qtbase5-dev libqt5svg5-dev libqt5websockets5-dev \
qtmultimedia5-dev libzip-dev libsodium-dev libgcrypt20-dev libx11-xcb-dev \
protobuf-compiler libprotobuf-dev libhidapi-dev libusb-dev libusb-1.0-0-dev

```

**3. Run the build:**

```bash
cd /feather
mkdir build-compat-test
cd build-compat-test
# This will trigger your new auto-detection logic
cmake ..
# Compile (use -j to speed it up)
make -j$(nproc)

```

If this succeeds, your CI job will succeed.

---

### **Option 2: Installing Qt 5 on Arch (Native Side-by-Side)**

Arch Linux provides Qt 5 packages that can coexist with Qt 6. You can force CMake to use them.

**1. Install Qt 5 base and modules:**

```bash
sudo pacman -S qt5-base qt5-svg qt5-wayland
yay -S qt5-websockets qt5-multimedia
```

**2. Force CMake to use Qt 5:**
You need to tell CMake to prefer the Qt 5 installation. You can do this by explicitly setting the path or just hoping your `find_package(Qt6 ... QUIET)` logic fails correctly (it might not if Qt 6 is also installed).

To verify the **fallback path** specifically, run cmake with the variable to disable Qt 6:

```bash
mkdir build-qt5
cd build-qt5
cmake -DCMAKE_DISABLE_FIND_PACKAGE_Qt6=TRUE ..
make -j$(nproc)

```

**Why Option 1 is better:**
Option 2 tests "Qt 5 on Arch" (which is Qt 5.15.12+). Option 1 tests "Qt 5 on Ubuntu 22.04" (which might be an older patch release of 5.15). The Docker method is the only way to guarantee you haven't used a symbol that exists in Arch's Qt 5 but is missing in Ubuntu's Qt 5.
