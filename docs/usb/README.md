# Synergy Through USB Documentation

## Prerequisites
- libusb-1.0 (required for USB communication)
- CMake (build system)

## Building the Project
### Linux/Mac
1. Install prerequisites:
   ```bash
   # Ubuntu/Debian
   sudo apt-get install libusb-1.0-0-dev cmake build-essential

   # macOS
   brew install libusb cmake
   ```

2. Build the project:
   ```bash
   mkdir build
   cd build
   cmake ..
   make
   ```

### Windows
1. Install prerequisites:
   - CMake (https://cmake.org/download/)
   - Visual Studio with C++ support
   - libusb (via vcpkg or manual installation)

2. Generate Visual Studio project:
   ```bash
   mkdir build
   cd build
   cmake ..
   ```
   Open the generated .sln file in Visual Studio and build.

## Device Setup
### Linux
1. Create udev rules for USB device access:
   ```bash
   sudo nano /etc/udev/rules.d/99-synergy-usb.rules
   ```
   Add the following content (replace VID/PID with your device's values):
   ```
   SUBSYSTEM=="usb", ATTR{idVendor}=="XXXX", ATTR{idProduct}=="YYYY", MODE="0666"
   ```

2. Reload udev rules:
   ```bash
   sudo udevadm control --reload-rules
   sudo udevadm trigger
   ```

### Windows
No special setup required if using libusb-win32 or WinUSB drivers.

## Usage
1. Device Connection
   - Connect USB device
   - Verify device enumeration:
     ```bash
     lsusb  # Linux/Mac
     ```

2. Application Configuration
   Required parameters:
   - Vendor ID (VID)
   - Product ID (PID)
   - Bulk IN endpoint
   - Bulk OUT endpoint

3. Connection Workflow
   - Application initializes USB context
   - Discovers device using VID/PID
   - Establishes connection using bulk endpoints
   - Handles data transfer

## Troubleshooting
1. Permission Issues
   - Verify udev rules (Linux)
   - Run with elevated privileges if needed

2. Connection Issues
   - Check device enumeration
   - Verify VID/PID match
   - Ensure endpoints are correctly configured

## API Reference
See [API Documentation](api.md) for detailed interface documentation.
