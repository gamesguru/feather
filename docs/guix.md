To get the exact same hash as the Feather Wallet maintainers (a **deterministic** or **reproducible** build), you cannot use standard tools like `cmake` or `docker` alone. You must use **GNU Guix**.

This process creates a bit-for-bit identical copy of the official release.

### **Important: You cannot do this *on* Termux**

To match the maintainers' hash, you must build on an **x86_64 Linux machine** (like Ubuntu or Debian).

* **Why?** The maintainers generate the official binaries using x86_64 servers.
* **The Conflict:** Termux runs on Android (ARM architecture). Even if you compiled the code successfully on Termux, the resulting binary would be completely different (different CPU instructions) and the hash would never match.
* **The Solution:** You must perform this build on a Linux PC (or a VPS), targeting the platform you want (Windows, Linux, or Android).

---

### **Step-by-Step Guide (Run on Linux PC)**

#### **1. Install GNU Guix**

On your Linux machine (Ubuntu/Debian/Fedora), install the Guix package manager.

```bash
# Download the installation script
wget https://git.savannah.gnu.org/cgit/guix.git/plain/etc/guix-install.sh

# Make it executable and run it (requires sudo)
chmod +x guix-install.sh
sudo ./guix-install.sh

```

#### **2. Start the Guix Daemon**

Ensure the build daemon is running:

```bash
sudo systemctl start guix-daemon

```

#### **3. Clone Feather Wallet**

```bash
git clone https://github.com/feather-wallet/feather.git
cd feather

```

#### **4. Run the Deterministic Build Command**

Use the exact script found in the `guix.yml` file you shared. This script spins up an isolated environment that matches the maintainers' environment exactly.

**To build the Linux version:**

```bash
# This downloads all dependencies and builds the app
# HOSTS defines the target (x86_64-linux-gnu = Linux Desktop)
HOSTS="x86_64-linux-gnu" ./contrib/guix/guix-build

```

**To build the Windows version:**

```bash
HOSTS="x86_64-w64-mingw32" ./contrib/guix/guix-build

```

**To build the macOS version:**

```bash
HOSTS="x86_64-apple-darwin" ./contrib/guix/guix-build

```

#### **5. Verify the Hash**

Once the build finishes (this will take a long time, often 1-2 hours), the output will be in:
`guix/guix-build-<version>/output/<target>/`

Run `sha256sum` on the resulting file:

```bash
sha256sum guix/guix-build-*/output/x86_64-linux-gnu/feather-*.AppImage

```

Compare this hash with the `SHA256SUMS.asc` file published on the [Feather Wallet Releases page](https://github.com/feather-wallet/feather/releases). They should match exactly.

---

### **Summary for Termux Users**

If your goal was to verify the wallet works safely on your phone:

1. **Do not build on the phone.** Build on a PC using the steps above.
2. **Copy the verified binary** to your phone (if you built the Android APK, usually target `aarch64-linux-android` if supported by their Guix setup).
3. **Run it.**

This is the only way to mathematically guarantee you are running the exact same code as the developers.
